from flask import Flask, request, jsonify, render_template_string
import datetime
import os
import uuid

app = Flask(__name__)

# Configuration
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
UPLOAD_FOLDER = os.path.join(BASE_DIR, 'received_payloads')
if not os.path.exists(UPLOAD_FOLDER): os.makedirs(UPLOAD_FOLDER)

# Mission Simulation State
mission_log = []
satellite_health = {
    "status": "IDLE",
    "battery": 85,
    "temp": 32.5,
    "storage_used": "12%"
}

# --- API Endpoints for Payload ---

@app.route('/telemetry', methods=['POST'])
def receive_telemetry():
    """Receive Housekeeping data from the satellite."""
    data = request.json
    satellite_health.update(data)
    mission_log.append(f"[{datetime.datetime.now().strftime('%H:%M:%S')}] Telemetry Received: {data}")
    return jsonify({"status": "ACK"}), 200

@app.route('/upload', methods=['POST'])
def upload_file():
    """Store incoming image data."""
    filename = f"payload_{datetime.datetime.now().strftime('%Y%m%d_%H%M%S')}.jpg"
    filepath = os.path.join(UPLOAD_FOLDER, filename)
    with open(filepath, "wb") as f:
        f.write(request.data)
    mission_log.append(f"[{datetime.datetime.now().strftime('%H:%M:%S')}] Image Downlinked: {filename}")
    return "OK", 200

# --- Dashboard for Demonstration ---

@app.route('/')
def dashboard():
    return render_template_string('''
 <!DOCTYPE html>
<html>
<head>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script> <!-- Chart.js Library -->
    <style>
    body { background-color: #121212; color: #e0e0e0; font-family: 'Roboto', sans-serif; }
    .dashboard-grid { 
        display: grid; 
        grid-template-columns: 250px 1fr 250px; 
        gap: 10px; padding: 10px; height: 95vh; 
    }
    .panel { background: #1e1e1e; border: 1px solid #333; padding: 15px; border-radius: 4px; }
    .gauge { font-size: 2em; color: #00bcd4; text-align: center; margin-bottom: 20px; }
    .chart-box { height: 300px; }
    .img-box { text-align: center; }
    img { width: 100%; border: 1px solid #444; }
    </style>
</head>
<body>
    <div class="dashboard-grid">
    <!-- ဘယ်ဘက်ခြမ်း: Gauges -->
    <div class="panel">
        <h3>SENSORS</h3>
        <div class="gauge">Battery<br>{{health.battery}}%</div>
        <div class="gauge">Temp<br>{{health.temp}}°C</div>
    </div>

    <!-- အလယ်: Graph & Imagery -->
    <div class="panel">
        <div class="chart-box"><canvas id="telemetryChart"></canvas></div>
        <div class="img-box">
            <h3>Payload Imagery</h3>
            <img src="/latest_image" alt="Satellite View">
        </div>
    </div>

    <!-- ညာဘက်: Status -->
    <div class="panel">
        <h3>MISSION STATUS</h3>
        <div style="color: #4caf50;">{{health.status}}</div>
    </div>
</div>
        
        <!-- Graph Section -->
        <div class="chart-container">
            <canvas id="batteryChart"></canvas>
        </div>

        <script>
            const ctx = document.getElementById('batteryChart').getContext('2d');
            const batteryChart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: ['T-5', 'T-4', 'T-3', 'T-2', 'T-1', 'Now'],
                    datasets: [{
                        label: 'Battery Level (%)',
                        data: [90, 88, 85, 87, 86, {{health.battery}}], // Flask မှလာသော Value
                        borderColor: '#00ff41',
                        fill: true
                    }]
                },
                options: { scales: { y: { beginAtZero: true, max: 100 } } }
            });
        </script>
    </div>
</body>
</html>
    ''', health=satellite_health, logs=mission_log[-10:])

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)