import requests

AI_URL = 'http://localhost:8001'

try:
    resp = requests.get(f'{AI_URL}/health', timeout=5)
    print(f'健康检查: {resp.status_code} {resp.text}')
except Exception as e:
    print(f'连接失败: {e}')
