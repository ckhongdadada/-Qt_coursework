"""
Qwen2.5-1.5B-Instruct 本地模型服务
提供 OpenAI 兼容 API，供 Qt 桌面应用调用
启动: python qwen_server.py
"""

import json
import time
import uuid
from pathlib import Path

import torch
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from transformers import AutoModelForCausalLM, AutoTokenizer
import uvicorn

# ── 配置 ──────────────────────────────────────────
MODEL_DIR = Path(__file__).parent / "qwen_models" / "Qwen" / "Qwen2___5-1___5B-Instruct"
HOST = "127.0.0.1"
PORT = 8001

# ── 加载模型 ──────────────────────────────────────
print(f"[qwen_server] 正在加载模型: {MODEL_DIR}")
tokenizer = AutoTokenizer.from_pretrained(str(MODEL_DIR), trust_remote_code=True)

device = "cuda" if torch.cuda.is_available() else "cpu"
print(f"[qwen_server] 使用设备: {device}")

model = AutoModelForCausalLM.from_pretrained(
    str(MODEL_DIR),
    torch_dtype=torch.float16 if device == "cuda" else torch.float32,
    device_map=device,
    trust_remote_code=True,
)
model.eval()
print(f"[qwen_server] 模型加载完成，监听 http://{HOST}:{PORT}")

# ── FastAPI ────────────────────────────────────────
app = FastAPI(title="Qwen Local Model Server")
app.add_middleware(CORSMiddleware, allow_origins=["*"], allow_methods=["*"], allow_headers=["*"])


# ── 请求模型 ───────────────────────────────────────
class ChatRequest(BaseModel):
    messages: list[dict]
    max_tokens: int = 512
    temperature: float = 0.7
    stream: bool = False


class AnalyzeRequest(BaseModel):
    type: str = "general"
    messages: list[dict] | None = None
    max_tokens: int = 512
    temperature: float = 0.5


# ── 生成回复 ───────────────────────────────────────
def generate_reply(prompt: str, max_tokens: int = 512, temperature: float = 0.7) -> str:
    inputs = tokenizer(prompt, return_tensors="pt").to(model.device)
    with torch.no_grad():
        outputs = model.generate(
            **inputs,
            max_new_tokens=max_tokens,
            temperature=temperature if temperature > 0 else 1.0,
            do_sample=temperature > 0,
            top_p=0.9,
            pad_token_id=tokenizer.eos_token_id,
        )
    new_tokens = outputs[0][inputs["input_ids"].shape[1]:]
    return tokenizer.decode(new_tokens, skip_special_tokens=True).strip()


def build_prompt(messages: list[dict]) -> str:
    """将 OpenAI 格式 messages 转为 Qwen chat prompt"""
    conversation = []
    for msg in messages:
        role = msg.get("role", "user")
        content = msg.get("content", "")
        conversation.append(f"<|im_start|>{role}\n{content}<|im_end|>")
    conversation.append("<|im_start|>assistant\n")
    return "\n".join(conversation)


# ── API 路由 ───────────────────────────────────────
@app.get("/health")
async def health():
    return {"model_loaded": True, "model_loading": False, "model": "Qwen2.5-1.5B-Instruct"}


@app.post("/v1/chat/completions")
async def chat_completions(req: ChatRequest):
    prompt = build_prompt(req.messages)
    reply_text = generate_reply(prompt, req.max_tokens, req.temperature)

    return {
        "id": f"chatcmpl-{uuid.uuid4().hex[:8]}",
        "object": "chat.completion",
        "created": int(time.time()),
        "model": "Qwen2.5-1.5B-Instruct",
        "choices": [
            {
                "index": 0,
                "message": {"role": "assistant", "content": reply_text},
                "finish_reason": "stop",
            }
        ],
        "usage": {"prompt_tokens": 0, "completion_tokens": 0, "total_tokens": 0},
    }


@app.post("/v1/analyze")
async def analyze(req: AnalyzeRequest):
    analysis_prompts = {
        "course": "你是一位学业规划顾问。请根据学生的课程数据，给出详细的课程学习建议和规划。请用中文回答。",
        "goal": "你是一位目标管理顾问。请根据学生的目标数据，给出目标达成策略和建议。请用中文回答。",
        "experience": "你是一位职业发展顾问。请根据学生的实践经历，给出经历提升建议。请用中文回答。",
        "career": "你是一位职业规划顾问。请根据学生的数据，给出职业规划建议。请用中文回答。",
        "resume": "你是一位简历优化顾问。请根据学生的数据，给出简历优化建议。请用中文回答。",
        "achievement": "你是一位成就评估顾问。请根据学生的成就数据，给出提升建议。请用中文回答。",
        "comprehensive": "你是一位综合学业顾问。请根据学生的全面数据，给出综合学业发展建议。请用中文回答。",
        "general": "你是一位综合学业顾问。请根据学生的数据，给出全面的学业发展建议。请用中文回答。",
    }

    system_prompt = analysis_prompts.get(req.type, analysis_prompts["general"])

    messages = [
        {"role": "system", "content": system_prompt},
    ]
    if req.messages:
        messages.extend(req.messages)
    else:
        messages.append({"role": "user", "content": f"请对类型为「{req.type}」的数据进行分析，给出详细建议。"})

    prompt = build_prompt(messages)
    reply_text = generate_reply(prompt, req.max_tokens, req.temperature)

    return {
        "id": f"analyze-{uuid.uuid4().hex[:8]}",
        "analysis": reply_text,
        "type": req.type,
        "aiPowered": True,
        "model": "Qwen2.5-1.5B-Instruct",
    }


# ── 启动 ───────────────────────────────────────────
if __name__ == "__main__":
    uvicorn.run(app, host=HOST, port=PORT)
