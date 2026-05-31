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


def generate_response(prompt, max_tokens=256, temperature=0.3):
    global model, tokenizer, model_loaded
    
    if not model_loaded:
        return None, "模型未加载"
    
    try:
        import torch
        
        messages = [
            {"role": "system", "content": "你是一个学业发展规划助手，帮助用户分析学业情况、提供选课建议、规划职业发展。请用简洁专业的语言回答问题。"},
            {"role": "user", "content": prompt}
        ]
        
        text = tokenizer.apply_chat_template(
            messages,
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
    
    response_text, error = generate_response(prompt, max_tokens, temperature)
    
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
    
    load_model()
    
    logger.info(f"AI服务已启动，监听端口 {AI_PORT}")
    logger.info("API端点:")
    logger.info(f"  - GET  http://localhost:{AI_PORT}/health")
    logger.info(f"  - GET  http://localhost:{AI_PORT}/v1/status")
    logger.info(f"  - POST http://localhost:{AI_PORT}/v1/chat/completions")
    logger.info(f"  - POST http://localhost:{AI_PORT}/v1/analyze")
    
    app.run(host='0.0.0.0', port=AI_PORT, debug=False, threaded=True)
