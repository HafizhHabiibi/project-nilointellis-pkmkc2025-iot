# set_webhook.py
import os
import requests
from dotenv import load_dotenv

load_dotenv()
token = os.getenv("TELEGRAM_BOT_TOKEN")
ngrok_url = "https://8e9f25b3c974.ngrok-free.app"  # ganti dengan URL dari ngrok

url = f"https://api.telegram.org/bot{token}/setWebhook?url={ngrok_url}/telegram"
resp = requests.get(url)
print(resp.json())