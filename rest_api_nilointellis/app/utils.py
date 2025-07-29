import requests
from flask import current_app

def send_notif(message):
    token = current_app.config['TELEGRAM_BOT_TOKEN']
    chat_id = current_app.config['CHAT_ID']
    url = f"https://api.telegram.org/bot8<BOT_TOKEN_MU_YGY/sendMessage"

    payload = {
        'chat_id': chat_id,
        'text' : message,
    }

    try:
        requests.post(url, data=payload)
    except Exception as e:
        print("Gagal kirim ke Telegram", e)