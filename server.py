import subprocess
import json
from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app) # Allows your teammate's UI to talk to this server

# This creates an exact "socket" for the API request your teammate planned
@app.route('/api/bus/<bus_id>/parking-zone', methods=['POST'])
def update_parking(bus_id):
    # 1. Get the parking zone data from the frontend UI
    data = request.get_json()
    zone = data.get('zone')

    # 2. Run your C engine invisibly in the background with utf-8 encoding
    process = subprocess.run(
        ['api_engine.exe', 'update_zone', str(bus_id), str(zone)], 
        capture_output=True, 
        text=True,
        encoding='utf-8',
        errors='ignore'
    )

    # 3. Take the JSON output from your C code and send it back to the UI
    try:
        c_output = json.loads(process.stdout)
        return jsonify(c_output)
    except:
        return jsonify({"error": "Failed to read C engine", "details": process.stdout})

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
    except:
        return jsonify({"error": "Failed to read C engine", "details": process.stdout})
    
if __name__ == '__main__':
    print(" Traffic Cop Server running on http://localhost:5000 ...")
    app.run(port=5000)