import requests
import os
import json
from flask import current_app

CHAT_ID_FILE = "chat_id.json"

def save_chat_id(chat_id):
    """Simpan chat_id pengguna ke file JSON jika belum ada."""
    if os.path.exists(CHAT_ID_FILE):
        try:
            with open(CHAT_ID_FILE, 'r') as f:
                chat_ids = json.load(f)
        except json.JSONDecodeError:
            chat_ids = []
    else:
        chat_ids = []

    if chat_id not in chat_ids:
        chat_ids.append(chat_id)
        with open(CHAT_ID_FILE, "w") as f:
            json.dump(chat_ids, f)

def get_all_chat_ids():
    """Ambil semua chat_id yang tersimpan."""
    if os.path.exists(CHAT_ID_FILE):
        try:
            with open(CHAT_ID_FILE, 'r') as f:
                return json.load(f)
        except json.JSONDecodeError:
            return []
    return []

def send_notif(message):
    """Kirim notifikasi ke semua pengguna yang sudah menyimpan chat_id."""
    token = current_app.config['TELEGRAM_BOT_TOKEN']
    url = f"https://api.telegram.org/bot{token}/sendMessage"
    chat_ids = get_all_chat_ids()

    for chat_id in chat_ids:
        payload = {
            'chat_id': chat_id,
            'text': message,
            'parse_mode': "Markdown"
        }

        try:
            requests.post(url, data=payload)
        except Exception as e:
            print(f"Gagal kirim ke {chat_id}:", e)

def sokap(chat_id):
    """Respon ketika user memulai bot dengan /start."""
    save_chat_id(chat_id)
    pesan = (
        "*👋📡 Selamat datang di NiloIntellisBOT!*\n\n"
        "PKM-KC Universitas Teknologi Yogyakarta 2025"
    )
    token = current_app.config['TELEGRAM_BOT_TOKEN']
    url = f"https://api.telegram.org/bot{token}/sendMessage"

    payload = {
        'chat_id': chat_id,
        'text': pesan,
        'parse_mode': "Markdown"
    }

    try:
        requests.post(url, data=payload)
    except Exception as e:
        print(f"Gagal kirim sambutan ke {chat_id}:", e)