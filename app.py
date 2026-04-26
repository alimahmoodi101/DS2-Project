from flask import Flask, request, jsonify, send_from_directory
import subprocess, json, os

app = Flask(__name__, static_folder='frontend')

BINARY       = "./pathfinder.exe"
COORDS_FILE  = "src/karachi_cluster_coords.json"

# Load cluster node coordinates (internal_id -> {lat, lon, osm})
coords = {}
if os.path.exists(COORDS_FILE):
    with open(COORDS_FILE) as f:
        coords = json.load(f)
    print(f"Loaded coordinates for {len(coords)} cluster nodes.")
else:
    print("WARNING: src/karachi_cluster_coords.json not found. Run setup_cluster.py first.")

CLUSTER_SIZE = 100   # valid node IDs: 0..99


@app.route('/')
def index():
    return send_from_directory('frontend', 'index.html')


@app.route('/api/path')
def find_path():
    try:
        start = int(request.args.get('start', 0))
        end   = int(request.args.get('end',   50))
    except ValueError:
        return jsonify({"error": "start and end must be integers"}), 400

    if not (0 <= start < CLUSTER_SIZE and 0 <= end < CLUSTER_SIZE):
        return jsonify({"error": f"Node IDs must be between 0 and {CLUSTER_SIZE-1}"}), 400

    if start == end:
        return jsonify({"error": "Start and end nodes must be different"}), 400

    try:
        result = subprocess.run(
            [BINARY, str(start), str(end)],
            capture_output=True, text=True, timeout=30
        )
        data = json.loads(result.stdout)
    except subprocess.TimeoutExpired:
        return jsonify({"error": "Query timed out"}), 504
    except Exception as e:
        return jsonify({"error": str(e)}), 500

    if data.get("error"):
        return jsonify(data), 400

    # Attach lat/lon for each node in path
    path_coords = []
    for internal_id in data.get("path", []):
        entry = coords.get(str(internal_id))
        if entry:
            path_coords.append({
                "lat": entry["lat"],
                "lon": entry["lon"],
                "internal": internal_id,
                "osm": entry.get("osm", "")
            })

    data["path_coords"] = path_coords
    return jsonify(data)


@app.route('/api/nodes')
def get_nodes():
    """Return all cluster nodes with coords for the frontend to display."""
    return jsonify(coords)


@app.route('/api/info')
def info():
    return jsonify({
        "cluster_size": CLUSTER_SIZE,
        "coords_loaded": len(coords),
        "map_ready": len(coords) > 0
    })


if __name__ == '__main__':
    app.run(debug=True, port=5000)