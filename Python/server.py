# It is the main entry point for the Traffic Cop server, 
# which acts as a bridge between the C++ engine and the web frontend.
# What this server does is to receive HTTP requests from the frontend,
# call the C++ engine with the appropriate arguments, 
# and return the results back to the frontend
# Import subprocess to execute the compiled C++ engine.
import subprocess
# Import json to parse the engine's JSON output.
import json
# Import Flask utilities to create API routes and return JSON responses.
from flask import Flask, request, jsonify
# Enable Cross-Origin Resource Sharing for frontend requests.
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

# Point to your compiled C++ engine executable
ENGINE_PATH = 'C_engine/transit_engine.exe'
# GET endpoint: Retrieve information about all buses.
# Calls the C++ engine using the get_all_buses command.
# Parses the engine's JSON output and returns it to the frontend.

@app.route('/api/buses', methods=['GET'])
def get_all_buses():
    process = subprocess.run(
        [ENGINE_PATH, 'get_all_buses'], 
        capture_output=True, 
        text=True,
        encoding='utf-8',
        errors='ignore'
    )
    try:
         # Convert the C++ engine's JSON output into a Flask response.
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        # Return diagnostic information if the output is invalid.
        return jsonify({"error": "Failed to parse C++ engine output", "details": process.stdout})
# GET endpoint: Retrieve information about all buses.
# Calls the C++ engine using the get_all_buses command.
# Parses the engine's JSON output and returns it to the frontend.
@app.route('/api/bus/<bus_no>', methods=['GET'])
def get_bus_info(bus_no):
    process = subprocess.run(
        [ENGINE_PATH, 'get_bus', str(bus_no)], 
        capture_output=True, 
        text=True,
        encoding='utf-8',
        errors='ignore'
    )
    try:
          # Return the parsed bus details as JSON.
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        # Report the engine output if JSON parsing fails.
        return jsonify({"error": "Failed to parse C++ engine output", "details": process.stdout})
# POST endpoint: Update the parking zone assigned to a bus.
# Reads the requested zone from the JSON request body.
# Sends the bus number and zone to the C++ engine.
@app.route('/api/bus/<bus_no>/parking-zone', methods=['POST'])
def update_parking(bus_no):
    data = request.get_json()
    zone = data.get('zone')
    process = subprocess.run(
        [ENGINE_PATH, 'update_zone', str(bus_no), str(zone)], 
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
        [ENGINE_PATH, 'update_seats', str(bus_no), str(seats)], 
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
        [ENGINE_PATH, 'advance_stop', str(bus_no)], 
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
        [ENGINE_PATH, 'end_trip', str(bus_no)], 
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
        [ENGINE_PATH, 'get_buses_by_stop', str(station_name)], 
        capture_output=True, 
        text=True,
        encoding='utf-8',
        errors='ignore'
    )
    try:
        return jsonify(json.loads(process.stdout))
    except Exception as e:
        return jsonify({"error": "Failed to parse C++ engine output", "details": process.stdout})

if __name__ == '__main__':
    print("🚦 Traffic Cop Server running on http://localhost:5000 ...")
    app.run(port=5000)
