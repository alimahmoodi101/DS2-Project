"""
Run this ONCE on your machine to fetch lat/lon for all graph nodes.
It saves src/karachi_nodes.json which the frontend uses to draw the map.

Usage:
    pip install requests
    python fetch_coords.py
"""

import requests
import json
import time

def fetch_coords():
    # Load all unique OSM node IDs from graph file
    osm_ids = set()
    with open('src/karachi_graph.txt') as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) >= 2:
                osm_ids.add(parts[0])
                osm_ids.add(parts[1])

    osm_ids = list(osm_ids)
    print(f"Total nodes to fetch: {len(osm_ids)}")

    coords = {}
    BATCH = 500  # fetch 500 nodes per request
    headers = {'User-Agent': 'DS2-Project/1.0 (student-project)'}
    url = "https://overpass-api.de/api/interpreter"

    for i in range(0, len(osm_ids), BATCH):
        batch = osm_ids[i:i+BATCH]
        ids_str = ','.join(batch)
        query = f'[out:json][timeout:60]; node(id:{ids_str}); out body;'

        for attempt in range(3):
            try:
                r = requests.post(url, data={'data': query}, headers=headers, timeout=65)
                if r.status_code == 200:
                    data = r.json()
                    for elem in data['elements']:
                        coords[str(elem['id'])] = [elem['lat'], elem['lon']]
                    break
                else:
                    print(f"  Batch {i//BATCH+1}: HTTP {r.status_code}, retrying...")
                    time.sleep(5)
            except Exception as e:
                print(f"  Batch {i//BATCH+1}: Error {e}, retrying...")
                time.sleep(5)

        progress = min(i + BATCH, len(osm_ids))
        print(f"  Fetched {progress}/{len(osm_ids)} nodes ({len(coords)} coords so far)")
        time.sleep(1.5)  # be polite to the API

    print(f"\nDone! Got coordinates for {len(coords)} / {len(osm_ids)} nodes")

    with open('src/karachi_nodes.json', 'w') as f:
        json.dump(coords, f)

    print("Saved to src/karachi_nodes.json")

if __name__ == "__main__":
    fetch_coords()