import subprocess
import json
from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

@app.route('/api/buses', methods=['GET'])
def get_all_buses():
    process = subprocess.run(
        ['api_engine.exe', 'get_all_buses'], 
        capture_output=True, 
        text=True,
        encoding='utf-8',
        errors='ignore'
    )
    try:
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        return jsonify({"error": "Failed to parse C engine output", "details": process.stdout})

@app.route('/api/bus/<bus_no>', methods=['GET'])
def get_bus_info(bus_no):
    process = subprocess.run(
        ['api_engine.exe', 'get_bus', str(bus_no)], 
        capture_output=True, 
        text=True,
        encoding='utf-8',
        errors='ignore'
    )
    try:
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        return jsonify({"error": "Failed to parse C engine output", "details": process.stdout})

@app.route('/api/bus/<bus_no>/parking-zone', methods=['POST'])
def update_parking(bus_no):
    data = request.get_json()
    zone = data.get('zone')
    process = subprocess.run(
        ['api_engine.exe', 'update_zone', str(bus_no), str(zone)], 
        capture_output=True, 
        text=True,
        encoding='utf-8',
        errors='ignore'
    )
    try:
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        return jsonify({"error": "Failed to update zone", "details": process.stdout})

@app.route('/api/bus/<bus_no>/seats', methods=['POST'])
def update_seats(bus_no):
    data = request.get_json()
    seats = data.get('available_seats')
    process = subprocess.run(
        ['api_engine.exe', 'update_seats', str(bus_no), str(seats)], 
        capture_output=True, 
        text=True, 
        encoding='utf-8', 
        errors='ignore'
    )
    try:
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        return jsonify({"error": "Failed to update seats", "details": process.stdout})

@app.route('/api/bus/<bus_no>/advance-stop', methods=['POST'])
def advance_stop(bus_no):
    process = subprocess.run(
        ['api_engine.exe', 'advance_stop', str(bus_no)], 
        capture_output=True, 
        text=True, 
        encoding='utf-8', 
        errors='ignore'
    )
    try:
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        return jsonify({"error": "Failed to advance stop", "details": process.stdout})

@app.route('/api/bus/<bus_no>/end-trip', methods=['POST'])
def end_trip(bus_no):
    process = subprocess.run(
        ['api_engine.exe', 'end_trip', str(bus_no)], 
        capture_output=True, 
        text=True, 
        encoding='utf-8', 
        errors='ignore'
    )
    try:
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        return jsonify({"error": "Failed to end trip", "details": process.stdout})

@app.route('/api/station/<station_name>', methods=['GET'])
def get_buses_by_station(station_name):
    process = subprocess.run(
        ['api_engine.exe', 'get_buses_by_stop', str(station_name)], 
        capture_output=True, 
        text=True,
        encoding='utf-8',
        errors='ignore'
    )
    try:
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        return jsonify({"error": "Failed to parse C engine output", "details": process.stdout})

if __name__ == '__main__':
    print("🚦 Traffic Cop Server running on http://localhost:5000 ...")
    app.run(port=5000)