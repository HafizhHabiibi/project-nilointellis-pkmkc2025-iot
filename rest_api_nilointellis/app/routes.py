import os
from dotenv import load_dotenv
from flask import Blueprint, request, jsonify, Response
from datetime import datetime, timedelta, timezone
from pymongo.mongo_client import MongoClient
from pymongo.server_api import ServerApi
from bson.json_util import dumps
import pytz

# Load environment variables
load_dotenv()

# Blueprint setup
api = Blueprint('api', __name__)

# MongoDB connection via .env
MONGO_URI = os.getenv("MONGO_URI")
client = MongoClient(MONGO_URI, server_api=ServerApi('1'))
db = client["nilo"]
collection = db["sensor"]

# Global variable untuk data terakhir
data_terakhir = {}

# fungsi konversi WIB
def konversi_wib(dt_utc):
    if not isinstance(dt_utc, datetime):
        return dt_utc
    if dt_utc.tzinfo is None:
        dt_utc = dt_utc.replace(tzinfo=timezone.utc)
    wib = pytz.timezone("Asia/Jakarta")
    return dt_utc.astimezone(wib).strftime('%Y-%m-%d %H:%M:%S')

# Endpoint utama
@api.route('/')
def home():
    return "✅ API IoT Aktif"

# POST data sensor
@api.route('/sensor', methods=['POST'])
def simpan_data():
    global data_terakhir
    data = request.get_json()

    if not data:
        return jsonify({"error": "Tidak ada data yang diterima"}), 400

    data_terakhir = data
    data_terakhir['timestamp'] = datetime.now(timezone.utc)
    collection.insert_one(data_terakhir)

    print("✅ Data berhasil disimpan:", data_terakhir)
    return jsonify({"message": "Data berhasil disimpan"}), 201

# GET data sensor terakhir
@api.route('/sensor', methods=['GET'])
def ambil_data():
    hasil_data = data_terakhir.copy()
    hasil_data.pop('_id', None)  # Hapus _id jika ada
    if 'timestamp' in hasil_data:
        hasil_data['timestamp'] = konversi_wib(hasil_data['timestamp'])
    return jsonify(hasil_data), 200

# GET riwayat data sensor
@api.route('/sensor/history', methods=['GET'])
def ambil_riwayat_data():
    start = request.args.get('start')
    end = request.args.get('end')

    query = {}
    if start and end:
        try:
            start_dt = datetime.strptime(start, "%Y-%m-%d")
            end_dt = datetime.strptime(end, "%Y-%m-%d") + timedelta(days=1)
            query['timestamp'] = {"$gte": start_dt, "$lt": end_dt}
        except ValueError:
            return jsonify({"error": "Format tanggal tidak valid. Gunakan YYYY-MM-DD"}), 400

    data = list(collection.find(query).sort('timestamp', -1).limit(100))

    for item in data:
        item.pop('_id', None)
        if 'timestamp' in item:
            item['timestamp'] = konversi_wib(item['timestamp'])

    return Response(dumps(data), mimetype='application/json')