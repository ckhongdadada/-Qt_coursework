"""
AI服务 - 为C++后端提供大模型推理能力
使用Flask提供REST API接口，支持Qwen2.5-1.5B模型
"""

import os
import sys
import json
import logging
import importlib.util
import pkgutil
from datetime import datetime

# Flask 2.x still calls pkgutil.get_loader(), which was removed in Python 3.14.
# Keep the server usable with the user's current Python 3.14 environment.
if not hasattr(pkgutil, 'get_loader'):
    def _compat_get_loader(name):
        module = sys.modules.get(name)
        loader = getattr(module, '__loader__', None)
        if loader is not None:
            return loader
        try:
            spec = importlib.util.find_spec(name)
        except (ImportError, ValueError):
            return None
        return spec.loader if spec else None

    pkgutil.get_loader = _compat_get_loader

from flask import Flask, request, jsonify
from flask_cors import CORS

logging.basicConfig(
    level=logging.INFO,
    format='[%(asctime)s] [%(levelname)s] %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)
logger = logging.getLogger(__name__)

app = Flask(__name__)
CORS(app)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_MODEL_ROOT = os.path.join(BASE_DIR, 'qwen_models')


def resolve_model_path(path):
    """Accept either qwen_models root or a concrete HuggingFace model directory."""
    model_root = os.path.abspath(path)
    if os.path.isfile(os.path.join(model_root, 'config.json')):
        return model_root

    preferred = os.path.join(model_root, 'Qwen', 'Qwen2___5-1___5B-Instruct')
    if os.path.isfile(os.path.join(preferred, 'config.json')):
        return preferred

    for root, _, files in os.walk(model_root):
        if 'config.json' in files and (
            'model.safetensors' in files
            or any(name.endswith('.safetensors') for name in files)
            or any(name.endswith('.bin') for name in files)
        ):
            return root

    return model_root


MODEL_ROOT = os.environ.get('AI_MODEL_PATH', DEFAULT_MODEL_ROOT)
MODEL_PATH = resolve_model_path(MODEL_ROOT)
AI_PORT = int(os.environ.get('AI_PORT', 8001))

model = None
tokenizer = None
model_loaded = False
model_loading = False


def load_model():
    global model, tokenizer, model_loaded, model_loading
    
    if model_loaded or model_loading:
        return
    
    model_loading = True
    if not os.path.isfile(os.path.join(MODEL_PATH, 'config.json')):
        logger.error(f"模型配置文件不存在: {os.path.join(MODEL_PATH, 'config.json')}")
        model_loaded = False
        model_loading = False
        return

    logger.info(f"正在加载模型: {MODEL_PATH}")
    
    try:
        from transformers import AutoTokenizer, AutoModelForCausalLM
        import torch
        
        tokenizer = AutoTokenizer.from_pretrained(
            MODEL_PATH,
            trust_remote_code=True
        )
        
        try:
            model = AutoModelForCausalLM.from_pretrained(
                MODEL_PATH,
                torch_dtype=torch.float16,
                device_map="auto",
                trust_remote_code=True
            )
        except ImportError as e:
            if 'accelerate' not in str(e).lower():
                raise
            logger.warning("未检测到 accelerate，改用 CPU 模式加载模型。")
            model = AutoModelForCausalLM.from_pretrained(
                MODEL_PATH,
                torch_dtype=torch.float16,
                trust_remote_code=True
            )
        
        model.eval()
        model_loaded = True
        logger.info("模型加载完成")
        
    except Exception as e:
        logger.error(f"模型加载失败: {e}")
        model_loaded = False
    finally:
        model_loading = False


def generate_response(prompt, max_tokens=256, temperature=0.3, messages=None):
    global model, tokenizer, model_loaded
    
    if not model_loaded:
        return None, "模型未加载"
    
    try:
        import torch
        
        # 优先使用完整的 messages 列表（包含 system 上下文）
        if messages and isinstance(messages, list) and len(messages) > 0:
            chat_messages = messages
        else:
            chat_messages = [
                {"role": "system", "content": "你是一个学业发展规划助手，帮助用户分析学业情况、提供选课建议、规划职业发展。请用简洁专业的语言回答问题。"},
                {"role": "user", "content": prompt}
            ]
        
        text = tokenizer.apply_chat_template(
            chat_messages,
            tokenize=False,
            add_generation_prompt=True
        )
        
        inputs = tokenizer([text], return_tensors="pt").to(model.device)
        
        with torch.no_grad():
            outputs = model.generate(
                **inputs,
                max_new_tokens=max_tokens,
                temperature=temperature,
                top_p=0.9,
                do_sample=temperature > 0,
                pad_token_id=tokenizer.eos_token_id
            )
        
        generated_ids = outputs[0][inputs.input_ids.shape[1]:]
        response = tokenizer.decode(generated_ids, skip_special_tokens=True)
        
        return response.strip(), None
        
    except Exception as e:
        logger.error(f"生成响应失败: {e}")
        return None, str(e)


@app.route('/health', methods=['GET'])
def health_check():
    return jsonify({
        "status": "ok",
        "model_loaded": model_loaded,
        "model_loading": model_loading,
        "model_path": MODEL_PATH,
        "timestamp": datetime.now().isoformat()
    })


@app.route('/v1/chat/completions', methods=['POST'])
def chat_completions():
    data = request.get_json()
    
    if not data:
        return jsonify({"error": "无效的请求数据"}), 400
    
    messages = data.get('messages', [])
    max_tokens = min(int(data.get('max_tokens', 256)), 512)
    temperature = float(data.get('temperature', 0.3))
    
    if not messages:
        return jsonify({"error": "缺少messages参数"}), 400
    
    # 提取最后一条 user 消息作为 fallback prompt
    last_message = messages[-1]
    prompt = last_message.get('content', '')
    
    if not prompt:
        return jsonify({"error": "消息内容为空"}), 400
    
    if not model_loaded:
        return jsonify({
            "error": "模型未加载",
            "model_loaded": False,
            "message": "请等待本地大模型加载完成后重试"
        }), 503
    
    # 传递完整的 messages 列表（包含 system 上下文 + user 消息）
    response_text, error = generate_response(prompt, max_tokens, temperature, messages=messages)
    
    if error:
        return jsonify({
            "error": error,
            "model_loaded": model_loaded
        }), 500
    
    return jsonify({
        "id": f"chatcmpl-{datetime.now().strftime('%Y%m%d%H%M%S')}",
        "object": "chat.completion",
        "created": int(datetime.now().timestamp()),
        "model": "Qwen2.5-1.5B-Instruct",
        "choices": [{
            "index": 0,
            "message": {
                "role": "assistant",
                "content": response_text
            },
            "finish_reason": "stop"
        }],
        "usage": {
            "prompt_tokens": len(prompt),
            "completion_tokens": len(response_text),
            "total_tokens": len(prompt) + len(response_text)
        }
    })


@app.route('/v1/analyze', methods=['POST'])
def analyze():
    data = request.get_json()
    
    if not data:
        return jsonify({"error": "无效的请求数据"}), 400
    
    analysis_type = data.get('type', 'general')
    
    if not model_loaded:
        return jsonify({
            "error": "模型未加载",
            "model_loaded": False
        }), 503
    
    if analysis_type == 'course':
        prompt = f"""请分析以下课程数据，给出学习建议：
{json.dumps(data.get('data', {}), ensure_ascii=False, indent=2)}

请从以下几个方面给出建议：
1. 课程学习优先级
2. 学习方法建议
3. 时间安排建议"""
        
    elif analysis_type == 'career':
        prompt = f"""请分析以下职业规划数据，给出发展建议：
{json.dumps(data.get('data', {}), ensure_ascii=False, indent=2)}

请从以下几个方面给出建议：
1. 职业发展方向
2. 技能提升建议
3. 短期和长期目标"""
        
    elif analysis_type == 'goal':
        prompt = f"""请分析以下目标数据，给出实现建议：
{json.dumps(data.get('data', {}), ensure_ascii=False, indent=2)}

请从以下几个方面给出建议：
1. 目标可行性分析
2. 实现路径建议
3. 潜在风险和应对措施"""
    else:
        prompt = f"""请分析以下学业发展数据，给出综合建议：
{json.dumps(data.get('data', {}), ensure_ascii=False, indent=2)}"""
    
    response_text, error = generate_response(prompt, max_tokens=384, temperature=0.3)
    
    if error:
        return jsonify({"error": error, "model_loaded": model_loaded}), 500
    
    return jsonify({
        "type": analysis_type,
        "analysis": response_text,
        "timestamp": datetime.now().isoformat()
    })


@app.route('/parse-pdf', methods=['POST'])
def parse_pdf():
    """解析 PDF 培养方案，提取按专业分组的课程信息和学分要求"""
    logger.info("收到 /parse-pdf 请求")
    if 'file' not in request.files:
        logger.warning("请求中未包含 file 字段")
        return jsonify({"error": "未上传文件"}), 400
    
    file = request.files['file']
    logger.info(f"上传文件: {file.filename}, size={file.content_length if hasattr(file, 'content_length') else 'unknown'}")
    if not file.filename.lower().endswith('.pdf'):
        return jsonify({"error": "仅支持 PDF 文件"}), 400
    
    try:
        import tempfile
        import os
        
        with tempfile.NamedTemporaryFile(delete=False, suffix='.pdf') as tmp:
            file.save(tmp.name)
            tmp_path = tmp.name
        logger.info(f"文件已保存到临时路径: {tmp_path}")
        
        text = ""
        try:
            import pdfplumber
            logger.info("开始用 pdfplumber 提取文本")
            with pdfplumber.open(tmp_path) as pdf:
                for page in pdf.pages:
                    page_text = page.extract_text()
                    if page_text:
                        text += page_text + "\n"
            logger.info(f"PDF 文本提取完成，长度: {len(text)}")
        except ImportError:
            try:
                from PyPDF2 import PdfReader
                reader = PdfReader(tmp_path)
                for page in reader.pages:
                    page_text = page.extract_text()
                    if page_text:
                        text += page_text + "\n"
            except ImportError:
                os.unlink(tmp_path)
                return jsonify({"error": "未安装 PDF 解析库，请运行: pip install pdfplumber"}), 500
        
        os.unlink(tmp_path)
        
        if not text.strip():
            return jsonify({"error": "无法从 PDF 中提取文本"}), 400
        
        # 提取补充PDF文本
        tongxiu_text = ''
        tongshi_text = ''
        
        supp_tongxiu = request.files.get('supplementary_tongxiu')
        if supp_tongxiu and supp_tongxiu.filename.lower().endswith('.pdf'):
            with tempfile.NamedTemporaryFile(delete=False, suffix='.pdf') as tmp2:
                supp_tongxiu.save(tmp2.name)
                supp_tmp2_path = tmp2.name
            try:
                import pdfplumber as pb2
                with pb2.open(supp_tmp2_path) as pdf:
                    for page in pdf.pages:
                        pt = page.extract_text()
                        if pt:
                            tongxiu_text += pt + '\n'
            except Exception:
                pass
            finally:
                if os.path.exists(supp_tmp2_path):
                    os.unlink(supp_tmp2_path)
        
        supp_tongshi = request.files.get('supplementary_tongshi')
        if supp_tongshi and supp_tongshi.filename.lower().endswith('.pdf'):
            with tempfile.NamedTemporaryFile(delete=False, suffix='.pdf') as tmp3:
                supp_tongshi.save(tmp3.name)
                supp_tmp3_path = tmp3.name
            try:
                import pdfplumber as pb2
                with pb2.open(supp_tmp3_path) as pdf:
                    for page in pdf.pages:
                        pt = page.extract_text()
                        if pt:
                            tongshi_text += pt + '\n'
            except Exception:
                pass
            finally:
                if os.path.exists(supp_tmp3_path):
                    os.unlink(supp_tmp3_path)
        
        # 使用正则表达式解析
        parse_method = "regex"
        result = parse_curriculum_structured(text)
        
        # 处理补充PDF
        if tongxiu_text or tongshi_text:
            supp_courses, supp_warnings = parse_supplementary_courses(tongxiu_text, tongshi_text)
            if supp_warnings:
                result["parse_warnings"] = result.get("parse_warnings", []) + supp_warnings
            if supp_courses:
                tongxiu_required_credits = sum(c["credits"] for c in supp_courses if c["section"] == "通修必修")
                tongxiu_elective_credits = sum(c["credits"] for c in supp_courses if c["section"] == "通修选修")
                tongshi_elective_credits = sum(c["credits"] for c in supp_courses if c["section"] == "通识选修")
                
                for major in result["majors"]:
                    existing_codes = {c["code"]: idx for idx, c in enumerate(major["courses"])}
                    for sc in supp_courses:
                        if sc["code"] in existing_codes:
                            idx = existing_codes[sc["code"]]
                            major["courses"][idx]["section"] = sc["section"]
                            major["courses"][idx]["type"] = sc.get("type", major["courses"][idx].get("type", "Required"))
                        else:
                            major["courses"].append(sc)
                            existing_codes[sc["code"]] = len(major["courses"]) - 1
                    
                    if "通修" in major["credit_requirements"]:
                        total_tongxiu = major["credit_requirements"].pop("通修")
                        if tongxiu_required_credits > 0:
                            major["credit_requirements"]["通修必修"] = tongxiu_required_credits
                        if tongxiu_elective_credits > 0:
                            major["credit_requirements"]["通修选修"] = tongxiu_elective_credits
                        if tongxiu_required_credits == 0 and tongxiu_elective_credits == 0:
                            major["credit_requirements"]["通修必修"] = total_tongxiu
                    
                    if "通识" in major["credit_requirements"]:
                        total_tongshi = major["credit_requirements"].pop("通识")
                        major["credit_requirements"]["通识选修"] = tongshi_elective_credits if tongshi_elective_credits > 0 else total_tongshi
                
                result["total_courses"] = sum(len(m["courses"]) for m in result["majors"])
                result["supplementary_imported"] = len(supp_courses)
        
        response_data = {
            "success": True,
            "text": text[:2000],
            "majors": result["majors"],
            "total": result.get("total_courses", sum(len(m["courses"]) for m in result["majors"])),
            "parse_method": parse_method
        }
        if "parse_warnings" in result:
            response_data["parse_warnings"] = result["parse_warnings"]
        if "supplementary_imported" in result:
            response_data["supplementary_imported"] = result["supplementary_imported"]
        return jsonify(response_data)
        
    except Exception as e:
        logger.error(f"PDF 解析失败: {e}")
        return jsonify({"error": f"PDF 解析失败: {str(e)}"}), 500


def parse_curriculum_structured(text):
    """解析培养方案文本，按专业分组，提取学分要求
    
    PDF结构说明：
    - 每个专业包含"二、课程学分要求"（概述表）和"九、XX专业教学计划表"（详细课程表）
    - 概述表包含通识/通修/专业等板块的学分要求
    - 教学计划表包含具体课程列表，按板块分组
    """
    import re
    
    code_pattern = re.compile(r'^[A-Z]{2,4}[-]?[0-9]{3,4}$', re.IGNORECASE)
    loose_code_pattern = re.compile(r'^[A-Z]{1,5}[-]?[0-9]{2,5}$', re.IGNORECASE)
    lines = text.split('\n')
    
    # 识别专业段落标题（通用匹配：任何包含"教学计划表"或"培养方案"的标题行）
    major_title_pattern = re.compile(
        r'(.{2,30}?)\s*(教学计划表|培养方案)',
    )
    # 排除明显不是专业名的行
    major_exclude = re.compile(r'^\d|课程|合计|说明|备注|总计|序号|代码|名称|学分|学期|类别|来华')
    
    # 学分要求模式
    total_credit_pattern = re.compile(r'课程学分为\s*(\d+)\s*学分')
    section_credit_pattern = re.compile(r'(必修课|选修课)\s*(合计|应选修)\s*(\d+)')
    inline_credit_pattern = re.compile(r'(必\s*修\s*课|选\s*修\s*课)\s*(\d+)\s*学分')
    
    # 板块关键词映射 - 更精确地识别板块
    section_keywords = {
        "学科基础必修": {"type": "Required", "section": "学科基础"},
        "学科基础选修": {"type": "Elective", "section": "学科基础"},
        "专业方向必修": {"type": "Required", "section": "专业方向"},
        "专业方向选修": {"type": "Elective", "section": "专业方向"},
        "公共基础必修": {"type": "Required", "section": "公共基础"},
        "公共基础选修": {"type": "Elective", "section": "公共基础"},
        "实践": {"type": "Practice", "section": "实践"},
    }
    
    majors = []
    current_major = None
    current_section_type = "Required"
    current_section_name = "学科基础"
    seen_codes = set()
    # 用于记忆最近的板块前缀
    last_section_prefix = ""
    # 标记是否在"课程学分要求"部分（概述表）
    in_credit_requirements = False
    # 标记是否在"专业教学计划表"部分（详细课程表）
    in_teaching_plan = False
    
    for i, line in enumerate(lines):
        line = line.strip()
        if not line:
            continue
        
        # 检测专业标题
        major_match = major_title_pattern.search(line)
        if major_match:
            # 保存上一个专业
            if current_major:
                majors.append(current_major)
            
            major_name = major_match.group(1).strip()
            # 排除明显不是专业名的行
            if major_exclude.search(major_name) or len(major_name) < 2:
                continue
            # 清理中文数字前缀
            major_name = re.sub(r'^[一二三四五六七八九十]+[、.]', '', major_name).strip()
            # 清理年份和多余后缀
            major_name = re.sub(r'[（(]\s*\d{4}\s*年?\s*[）)]', '', major_name).strip()
            major_name = major_name.replace("教学计划表", "").replace("培养方案", "").strip()
            # 排除清理后过短或以数字/标点开头的无效专业名
            if len(major_name) < 2 or re.match(r'^[（(（\d]', major_name):
                continue
            
            current_major = {
                "name": major_name,
                "total_credits": 0,
                "credit_requirements": {},
                "courses": []
            }
            seen_codes = set()
            last_section_prefix = ""
            current_section_type = "Required"
            current_section_name = "学科基础"
            in_teaching_plan = True
            in_credit_requirements = False
            continue
        
        # 检测"课程学分要求"部分
        if '课程学分要求' in line:
            in_credit_requirements = True
            in_teaching_plan = False
            continue
        
        # 如果还没进入任何专业，跳过
        if not current_major:
            continue
        
        # ===== 在"课程学分要求"部分：提取学分要求 =====
        if in_credit_requirements:
            # 检测总学分
            total_match = total_credit_pattern.search(line)
            if total_match:
                current_major["total_credits"] = int(total_match.group(1))
                continue
            
            # 检测通识课程学分（如"通识课程（14学分）"）
            tongs_match = re.search(r'通识课程\s*[（(](\d+)\s*学分[）)]', line)
            if tongs_match:
                current_major["credit_requirements"]["通识选修"] = int(tongs_match.group(1))
                last_section_prefix = "通识"
                continue
            
            # 检测通修课程学分（如"通修课程（70学分）"）
            tongxiu_match = re.search(r'通修课程\s*[（(](\d+)\s*学分[）)]', line)
            if tongxiu_match:
                current_major["credit_requirements"]["通修"] = int(tongxiu_match.group(1))
                last_section_prefix = "通修"
                continue
            
            # 检测学科基础课程学分（如"学科基础课程（45学分）"）
            xkjc_match = re.search(r'学科基础课程\s*[（(](\d+)\s*学分[）)]', line)
            if xkjc_match:
                last_section_prefix = "学科基础"
                continue
            
            # 检测专业方向课程学分
            zyfx_match = re.search(r'专业方向课程\s*[（(](\d+)\s*学分[）)]', line)
            if zyfx_match:
                last_section_prefix = "专业方向"
                continue
            
            # 检测 "必修课 N" 或 "选修课 N" 行
            req_match = re.match(r'^(必\s*修\s*课|选\s*修\s*课)\s+(\d+)$', line)
            if req_match and last_section_prefix:
                course_type = re.sub(r'\s+', '', req_match.group(1)).replace("课", "")
                credit_val = int(req_match.group(2))
                req_key = last_section_prefix + course_type
                if req_key not in current_major["credit_requirements"]:
                    current_major["credit_requirements"][req_key] = credit_val
                continue
            
            # 检测 "其中：必修课49学分；选修课6学分" 格式
            if '其中' in line:
                inline_matches = inline_credit_pattern.findall(line)
                for course_type, credit_str in inline_matches:
                    credit_val = int(credit_str)
                    if credit_val > 0 and last_section_prefix:
                        normalized_type = re.sub(r'\s+', '', course_type).replace("课", "")
                        req_key = last_section_prefix + normalized_type
                        if req_key not in current_major["credit_requirements"]:
                            current_major["credit_requirements"][req_key] = credit_val
                continue
            
            # 检测实践教学学分
            sj_match = re.search(r'实践教学.*?(\d+)\s*学分', line)
            if sj_match:
                current_major["credit_requirements"]["实践"] = int(sj_match.group(1))
                continue
            
            # 如果遇到"三、学制"等标题，说明学分要求部分结束
            if re.match(r'^[三四五六七八九十]+[、.]', line):
                in_credit_requirements = False
                continue
            
            continue
        
        # ===== 在"专业教学计划表"部分：提取课程 =====
        
        # 检测总学分要求（教学计划表中也可能出现）
        total_match = total_credit_pattern.search(line)
        if total_match:
            current_major["total_credits"] = int(total_match.group(1))
            continue
        
        # 检测板块合计行（如"学科基础必修课合计 512 32"）
        section_match = section_credit_pattern.search(line)
        if section_match:
            section_name = section_match.group(0).split("合计")[0].split("应选修")[0].strip()
            credit_val = int(section_match.group(3))
            stripped = re.sub(r'\s+', '', section_name.replace("课", "").replace("合计", ""))
            if stripped not in ("必修", "选修"):
                req_key = stripped
                if req_key not in current_major["credit_requirements"]:
                    current_major["credit_requirements"][req_key] = credit_val
            continue
        
        # 检测 "学科基础选修课应选修13学分" 格式
        elective_req_match = re.search(r'(学科基础|专业方向|公共基础).*?选修课.*?应选修\s*(\d+)\s*学分', line)
        if elective_req_match:
            prefix = elective_req_match.group(1)
            credit_val = int(elective_req_match.group(2))
            req_key = prefix + "选修"
            if req_key not in current_major["credit_requirements"]:
                current_major["credit_requirements"][req_key] = credit_val
            continue
        
        # 跳过表头、注释、描述
        if '课程代码' in line and '课程名称' in line:
            continue
        if '类别' in line and '课程代码' in line:
            continue
        if '合计' in line or '注：' in line or line.startswith('注:'):
            continue
        if '培养目标' in line or '毕业要求' in line or '培养方案' in line:
            continue
        if '学制' in line or '实行学分制' in line:
            continue
        if '课程类别' in line and '课程组' in line:
            continue
        # 跳过纯数字行（页码）
        if re.match(r'^\d+$', line):
            continue
        
        # 检测板块标题（如"学科基础必修课"、"专业方向必修课"等）
        line_has_code = any(code_pattern.match(p.strip()) for p in line.split())
        is_section_header = False
        if not line_has_code and '其中' not in line:
            for keyword, info in section_keywords.items():
                if keyword in line and len(line) < 30:
                    current_section_type = info["type"]
                    current_section_name = info["section"]
                    is_section_header = True
                    break
        if is_section_header:
            continue
        
        # 处理跨行的板块前缀（如"学科" + "基础" + "必修课"）
        prefix_keywords = ["学科", "基础", "专业", "方向", "公共"]
        for pk in prefix_keywords:
            if line == pk or line.startswith(pk + " "):
                if pk in ["学科", "专业", "公共"]:
                    current_section_type = "Required"
                    if pk == "公共":
                        current_section_name = "公共基础"
                    elif pk == "专业":
                        current_section_name = "专业方向"
                    elif pk == "学科":
                        current_section_name = "学科基础"
                remaining = line[len(pk):].strip()
                if remaining:
                    line = remaining
                else:
                    continue
                break
        
        # 检测"必修课"/"选修课"
        if line.startswith("必修课") or line.startswith("选修课"):
            is_req = line.startswith("必修课")
            current_section_type = "Required" if is_req else "Elective"
            type_word = "必修课" if is_req else "选修课"
            remaining = line[len(type_word):].strip()
            if remaining:
                line = remaining
            else:
                continue
        
        # 检测子组类别（如"通用类"、"专业类"、"计算机类"、"大数据类"等）
        if re.match(r'^.{2,6}类$', line):
            continue
        
        # 解析课程行
        parts = re.split(r'\s{2,}|\t', line)
        if len(parts) < 3:
            parts = line.split()
            if len(parts) < 3:
                continue
        
        # 必须包含有效课程代码
        has_valid_code = any(loose_code_pattern.match(p.strip()) for p in parts)
        if not has_valid_code:
            continue
        
        course = {
            "code": "",
            "name": "",
            "credits": 0,
            "semester": "",
            "type": current_section_type,
            "section": current_section_name,
            "status": "Planned"
        }
        
        name_parts = []
        for part in parts:
            part = part.strip()
            if not part:
                continue
            
            if loose_code_pattern.match(part) and not course["code"]:
                course["code"] = part.upper()
            elif re.match(r'^\d+(\.\d+)?$', part):
                val = float(part)
                if 0 < val <= 10 and course["credits"] == 0:
                    course["credits"] = val
                elif 1 <= val <= 8 and not course["semester"]:
                    course["semester"] = str(int(val))
            elif re.match(r'^第[一二三四五六七八九十]+学期$', part):
                course["semester"] = part
            elif part in ["通用类", "专业类", "创业类", "必修", "选修", "选修课", "必修课",
                          "计算机类", "大数据类", "人工智能类", "统计类", "经管贸易类",
                          "荣誉课程", "经管法", "经济类", "管理类", "法学类"]:
                pass
            else:
                if not course["code"]:
                    name_parts.append(part)
                elif not course["name"]:
                    course["name"] = part
        
        if not course["name"] and name_parts:
            course["name"] = " ".join(name_parts)
        
        if course["name"] and len(course["name"]) >= 2:
            if not course["code"]:
                course["code"] = f"CUR{len(seen_codes)+1:04d}"
            if course["code"] not in seen_codes:
                seen_codes.add(course["code"])
                current_major["courses"].append(course)
    
    # 保存最后一个专业
    if current_major:
        majors.append(current_major)
    
    # 合并同名专业（概述页+教学计划表页）
    merged_majors = {}
    for major in majors:
        name = major["name"]
        if name not in merged_majors:
            merged_majors[name] = major
        else:
            existing = merged_majors[name]
            # 保留总学分要求较大的那个
            if major["total_credits"] > existing["total_credits"]:
                existing["total_credits"] = major["total_credits"]
            # 合并学分要求（保留较大值）
            for key, val in major["credit_requirements"].items():
                if key not in existing["credit_requirements"] or val > existing["credit_requirements"][key]:
                    existing["credit_requirements"][key] = val
            # 合并课程（去重）
            existing_codes = {c["code"] for c in existing["courses"]}
            for course in major["courses"]:
                if course["code"] not in existing_codes:
                    existing["courses"].append(course)
                    existing_codes.add(course["code"])
    
    majors = list(merged_majors.values())
    total_courses = sum(len(m["courses"]) for m in majors)
    
    # 后处理：从课程列表中自动统计各板块学分要求，补全缺失的学分要求
    for major in majors:
        if not major["courses"]:
            continue
        
        # 按section分组统计课程总学分
        section_credits = {}
        for course in major["courses"]:
            section = course.get("section", "")
            if not section:
                continue
            section_credits[section] = section_credits.get(section, 0) + course.get("credits", 0)
        
        # 补全缺失的学分要求：用课程总学分作为该板块的学分要求
        for section, credits in section_credits.items():
            if section not in major["credit_requirements"]:
                major["credit_requirements"][section] = credits
        
        # 如果total_credits为0，用各板块学分之和作为总学分
        if major["total_credits"] == 0 and section_credits:
            major["total_credits"] = sum(section_credits.values())
    
    # 解析质量提示
    parse_warnings = []
    if len(majors) == 0:
        parse_warnings.append("未识别到任何专业。请确认PDF包含'教学计划表'或'培养方案'标题。")
    for major in majors:
        if major["total_credits"] == 0 and not major["credit_requirements"]:
            parse_warnings.append(f"专业「{major['name']}」：未识别到学分要求，请检查PDF格式。")
        if len(major["courses"]) == 0:
            parse_warnings.append(f"专业「{major['name']}」：未识别到课程信息。")
    
    result = {"majors": majors, "total_courses": total_courses}
    if parse_warnings:
        result["parse_warnings"] = parse_warnings
    return result


def parse_supplementary_courses(tongxiu_text, tongshi_text):
    """解析通修和通识补充PDF，提取课程列表"""
    import re
    courses = []
    parse_warnings = []
    
    code_pattern = re.compile(r'^[A-Z]{2,4}[-]?[0-9]{3,4}$', re.IGNORECASE)
    # 更宽松的课程代码匹配（支持更多格式如 CS1001, MAT-201, EE3010）
    loose_code_pattern = re.compile(r'^[A-Z]{1,5}[-]?[0-9]{2,5}$', re.IGNORECASE)
    
    # 解析通修课程
    if tongxiu_text:
        current_type = "Required"  # 默认必修
        current_section = "通修必修"
        total_lines = 0
        matched_lines = 0
        for line in tongxiu_text.split('\n'):
            line = line.strip()
            if not line:
                continue
            total_lines += 1
            # 检测板块标题（如"必修课"、"选修课"，也支持"必修课程"、"选修课程"）
            if re.match(r'^必\s*修\s*(课|课程)?$', line) and len(line) < 15:
                current_type = "Required"
                current_section = "通修必修"
                continue
            if re.match(r'^选\s*修\s*(课|课程)?$', line) and len(line) < 15:
                current_type = "Elective"
                current_section = "通修选修"
                continue
            # 尝试多种格式匹配课程行
            # 格式1: 代码 名称 必修/选修 学时 学分
            m = re.match(r'([A-Z]{1,5}[-]?\d{2,5})\s+(.+?)\s+(必修|选修)\s+(\d+)\s+(\d+)', line, re.IGNORECASE)
            if m:
                code, name, ctype, hours, credits = m.groups()
                courses.append({
                    "code": code.upper(),
                    "name": name.strip(),
                    "credits": int(credits),
                    "type": "Required" if ctype == "必修" else "Elective",
                    "section": "通修必修" if ctype == "必修" else "通修选修",
                    "category": "Required" if ctype == "必修" else "Elective"
                })
                matched_lines += 1
                continue
            # 格式2: 代码 名称 学时 学分（使用当前板块类型）
            m = re.match(r'([A-Z]{1,5}[-]?\d{2,5})\s+(.+?)\s+(\d+)\s+(\d+)', line, re.IGNORECASE)
            if m:
                code, name, hours, credits = m.groups()
                courses.append({
                    "code": code.upper(),
                    "name": name.strip(),
                    "credits": int(credits),
                    "type": current_type,
                    "section": current_section,
                    "category": current_type
                })
                matched_lines += 1
                continue
            # 格式3: 代码 名称 学分（无学时）
            m = re.match(r'([A-Z]{1,5}[-]?\d{2,5})\s+(.+?)\s+(\d+)\s*$', line, re.IGNORECASE)
            if m:
                code, name, credits = m.groups()
                # 最后一个数字可能是学分（1-10范围）
                val = int(credits)
                if 1 <= val <= 10:
                    courses.append({
                        "code": code.upper(),
                        "name": name.strip(),
                        "credits": val,
                        "type": current_type,
                        "section": current_section,
                        "category": current_type
                    })
                    matched_lines += 1
                    continue
            # 格式4: 更宽松的匹配（制表符或多空格分隔）
            parts = re.split(r'\s{2,}|\t', line)
            if len(parts) >= 4:
                has_code = any(loose_code_pattern.match(p.strip()) for p in parts)
                if has_code:
                    code = ""
                    name = ""
                    credits_val = 0
                    for p in parts:
                        p = p.strip()
                        if loose_code_pattern.match(p) and not code:
                            code = p.upper()
                        elif re.match(r'^\d+(\.\d+)?$', p) and credits_val == 0:
                            credits_val = float(p)
                        elif not name and p not in ["必修", "选修", "必修课", "选修课", "必修课程", "选修课程"]:
                            name = p
                    if code and name:
                        courses.append({
                            "code": code,
                            "name": name.strip(),
                            "credits": int(credits_val) if credits_val > 0 else 2,
                            "type": current_type,
                            "section": current_section,
                            "category": current_type
                        })
                        matched_lines += 1
        if total_lines > 0 and matched_lines == 0:
            parse_warnings.append(f"通修PDF：共{total_lines}行，未识别到课程。请检查PDF格式是否为表格形式。")
        elif total_lines > 0 and matched_lines < total_lines * 0.3:
            parse_warnings.append(f"通修PDF：共{total_lines}行，仅匹配{matched_lines}行课程。部分格式可能未被识别。")
    
    # 解析通识课程
    if tongshi_text:
        current_type = "Elective"
        current_section = "通识选修"
        total_lines = 0
        matched_lines = 0
        for line in tongshi_text.split('\n'):
            line = line.strip()
            if not line:
                continue
            total_lines += 1
            # 格式1: 代码 名称 选修 学时 学分
            m = re.match(r'([A-Z]{1,5}[-]?\d{2,5})\s+(.+?)\s+选修\s+(\d+)\s+(\d+)', line, re.IGNORECASE)
            if m:
                code, name, hours, credits = m.groups()
                courses.append({
                    "code": code.upper(),
                    "name": name.strip(),
                    "credits": int(credits),
                    "type": "Elective",
                    "section": "通识选修",
                    "category": "Elective"
                })
                matched_lines += 1
                continue
            # 格式2: 代码 名称 学时 学分
            m = re.match(r'([A-Z]{1,5}[-]?\d{2,5})\s+(.+?)\s+(\d+)\s+(\d+)', line, re.IGNORECASE)
            if m:
                code, name, hours, credits = m.groups()
                courses.append({
                    "code": code.upper(),
                    "name": name.strip(),
                    "credits": int(credits),
                    "type": "Elective",
                    "section": "通识选修",
                    "category": "Elective"
                })
                matched_lines += 1
                continue
            # 格式3: 代码 名称 学分
            m = re.match(r'([A-Z]{1,5}[-]?\d{2,5})\s+(.+?)\s+(\d+)\s*$', line, re.IGNORECASE)
            if m:
                code, name, credits = m.groups()
                val = int(credits)
                if 1 <= val <= 10:
                    courses.append({
                        "code": code.upper(),
                        "name": name.strip(),
                        "credits": val,
                        "type": "Elective",
                        "section": "通识选修",
                        "category": "Elective"
                    })
                    matched_lines += 1
                    continue
            # 格式4: 宽松匹配
            parts = re.split(r'\s{2,}|\t', line)
            if len(parts) >= 4:
                has_code = any(loose_code_pattern.match(p.strip()) for p in parts)
                if has_code:
                    code = ""
                    name = ""
                    credits_val = 0
                    for p in parts:
                        p = p.strip()
                        if loose_code_pattern.match(p) and not code:
                            code = p.upper()
                        elif re.match(r'^\d+(\.\d+)?$', p) and credits_val == 0:
                            credits_val = float(p)
                        elif not name and p not in ["选修", "选修课", "选修课程"]:
                            name = p
                    if code and name:
                        courses.append({
                            "code": code,
                            "name": name.strip(),
                            "credits": int(credits_val) if credits_val > 0 else 2,
                            "type": "Elective",
                            "section": "通识选修",
                            "category": "Elective"
                        })
                        matched_lines += 1
        if total_lines > 0 and matched_lines == 0:
            parse_warnings.append(f"通识PDF：共{total_lines}行，未识别到课程。请检查PDF格式是否为表格形式。")
        elif total_lines > 0 and matched_lines < total_lines * 0.3:
            parse_warnings.append(f"通识PDF：共{total_lines}行，仅匹配{matched_lines}行课程。部分格式可能未被识别。")
    
    return courses, parse_warnings


@app.route('/parse_supplementary', methods=['POST'])
def parse_supplementary_endpoint():
    """解析补充PDF（通修+通识），返回课程列表"""
    try:
        tongxiu_path = request.form.get('tongxiu_path', '')
        tongshi_path = request.form.get('tongshi_path', '')
        
        tongxiu_text = ''
        tongshi_text = ''
        
        import pdfplumber
        
        if tongxiu_path and os.path.exists(tongxiu_path):
            with pdfplumber.open(tongxiu_path) as pdf:
                for page in pdf.pages:
                    page_text = page.extract_text()
                    if page_text:
                        tongxiu_text += page_text + '\n'
        
        if tongshi_path and os.path.exists(tongshi_path):
            with pdfplumber.open(tongshi_path) as pdf:
                for page in pdf.pages:
                    page_text = page.extract_text()
                    if page_text:
                        tongshi_text += page_text + '\n'
        
        courses, warnings = parse_supplementary_courses(tongxiu_text, tongshi_text)
        
        return jsonify({
            "success": True,
            "courses": courses,
            "warnings": warnings,
            "total": len(courses)
        })
        
    except Exception as e:
        logger.error(f"补充 PDF 解析失败: {e}")
        return jsonify({"error": f"补充 PDF 解析失败: {str(e)}"}), 500


@app.route('/v1/status', methods=['GET'])
def get_status():
    return jsonify({
        "model_loaded": model_loaded,
        "model_loading": model_loading,
        "model_path": MODEL_PATH,
        "model_name": "Qwen2.5-1.5B-Instruct",
        "server": "ai_server.py",
        "port": AI_PORT
    })


if __name__ == '__main__':
    logger.info("=" * 50)
    logger.info("AI服务启动中...")
    logger.info(f"模型路径: {MODEL_PATH}")
    logger.info(f"服务端口: {AI_PORT}")
    logger.info("=" * 50)
    
    if os.environ.get('AI_LOAD_MODEL', '0') == '1':
        load_model()
    else:
        logger.info("跳过本地大模型加载（PDF解析使用正则，无需模型）")
    
    logger.info(f"AI服务已启动，监听端口 {AI_PORT}")
    logger.info("API端点:")
    logger.info(f"  - GET  http://localhost:{AI_PORT}/health")
    logger.info(f"  - GET  http://localhost:{AI_PORT}/v1/status")
    logger.info(f"  - POST http://localhost:{AI_PORT}/v1/chat/completions")
    logger.info(f"  - POST http://localhost:{AI_PORT}/v1/analyze")
    logger.info(f"  - POST http://localhost:{AI_PORT}/parse-pdf")
    
    app.run(host='0.0.0.0', port=AI_PORT, debug=False, threaded=True)
