import subprocess
import json
from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

@app.route('/api/bus/<bus_id>', methods=['GET'])
def get_bus_info(bus_id):
    process = subprocess.run(
        ['api_engine.exe', 'get_bus', str(bus_id)], 
        capture_output=True, 
        text=True,
        encoding='utf-8',
        errors='ignore'
    )
    try:
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        return jsonify({"error": "Failed to parse C engine output", "details": process.stdout})

@app.route('/api/bus/<bus_id>/parking-zone', methods=['POST'])
def update_parking(bus_id):
    data = request.get_json()
    zone = data.get('zone')
    process = subprocess.run(
        ['api_engine.exe', 'update_zone', str(bus_id), str(zone)], 
        capture_output=True, 
        text=True,
        encoding='utf-8',
        errors='ignore'
    )
    try:
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        return jsonify({"error": "Failed to update zone", "details": process.stdout})

if __name__ == '__main__':
    print("🚦 Traffic Cop Server running on http://localhost:5000 ...")
    app.run(port=5000)