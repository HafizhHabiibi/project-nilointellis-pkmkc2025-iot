from flask import Flask, request, jsonify

app = Flask(__name__)

data_terakhir = {}

@app.route('/')
def home():
    return "✅ API AIoT Aktif!"

# === Sensor ===
@app.route('/sensor', methods=['POST'])
def simpan_data():
    global data_terakhir
    data = request.get_json()
    if not data:
        return jsonify({"error": "Tidak ada data yang dikirim"})
    data_terakhir = data
    print("Data sensor diterima: ", data_terakhir)
    return jsonify({"message": "Data berhasil disimpan"}), 201

@app.route('/sensor', methods=['GET'])
def ambil_data():
    return jsonify(data_terakhir), 200

# === Jalankan Server ===
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)