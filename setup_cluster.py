"""
Run this ONCE on your machine to set up the 100-node cluster.

Steps:
  1. Reads karachi_graph.txt locally, picks 100 tight nodes via BFS,
     writes src/cluster_graph.txt  (no internet needed for this part)

  2. Fetches lat/lon + name tags from Overpass API for all 100 nodes.

  3. For nodes with no name tag, calls Nominatim reverse-geocode to get
     the nearest street/area name so every node has a human-readable label.

Output:
  src/cluster_graph.txt           — the 100-node subgraph (internal IDs 0-99)
  src/karachi_cluster_coords.json — {internal_id: {lat, lon, osm, name, label}}

Usage:
    pip install requests
    python setup_cluster.py
"""

from fileinput import filename
from unittest import result

import requests, json, time, os
from collections import defaultdict, deque

GRAPH_FILE   = os.path.join('src', 'karachi_graph.txt')
COORDS_OUT   = os.path.join('src', 'karachi_cluster_coords.json')
GRAPH_OUT    = os.path.join('src', 'cluster_graph.txt')
CLUSTER_SIZE = 100

OVERPASS_MIRRORS = [
    'https://overpass-api.de/api/interpreter',
    'https://lz4.overpass-api.de/api/interpreter',
    'https://overpass.kumi.systems/api/interpreter',
]
NOMINATIM_URL = 'https://nominatim.openstreetmap.org/reverse'
HEADERS = {'User-Agent': 'DS2-Project/1.0 (student-project)'}


# ── Step 1: Build cluster from karachi_graph.txt ─────────────────────────────

def build_cluster():
    print(f"[1/3] Reading {GRAPH_FILE} and building 100-node cluster...")
    adj = defaultdict(list)

    with open(GRAPH_FILE) as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) >= 3:
                u, v, w = parts[0], parts[1], float(parts[2])
                adj[u].append((v, w))
                adj[v].append((u, w))

    # Seed = most-connected node (most likely to be a central intersection)
    seed = max(adj, key=lambda n: len(adj[n]))
    print(f"     Seed: OSM {seed} (degree {len(adj[seed])})")

    # BFS expanding to shortest-edge neighbors first → geographically tight cluster
    visited, order, queue = {}, [], deque([seed])
    visited[seed] = 0
    order.append(seed)

    while queue and len(order) < CLUSTER_SIZE:
        node = queue.popleft()
        for nbr, w in sorted(adj[node], key=lambda x: x[1]):
            if nbr not in visited and len(order) < CLUSTER_SIZE:
                visited[nbr] = len(order)
                order.append(nbr)
                queue.append(nbr)

    # Write cluster_graph.txt with internal IDs
    cluster_set = set(order)
    seen, lines = set(), []
    for u_osm in order:
        for v_osm, w in adj[u_osm]:
            if v_osm in cluster_set:
                key = tuple(sorted([u_osm, v_osm]))
                if key not in seen:
                    seen.add(key)
                    lines.append(f"{visited[u_osm]} {visited[v_osm]} {w:.2f}")

    with open(GRAPH_OUT, 'w') as f:
        f.write('\n'.join(lines))

    print(f"     {len(order)} nodes, {len(lines)} edges → {GRAPH_OUT}")
    return order   # OSM IDs in internal-ID order (index = internal ID)


# ── Step 2: Fetch lat/lon + name tags from Overpass ──────────────────────────

def fetch_overpass(osm_ids):
    print(f"\n[2/3] Fetching coordinates + name tags from Overpass API...")
    ids_str = ','.join(osm_ids)
    # "out body" returns full tags including name
    query = f'[out:json][timeout:30]; node(id:{ids_str}); out body;'

    for url in OVERPASS_MIRRORS:
        try:
            r = requests.post(url, data={'data': query}, headers=HEADERS, timeout=35)
            if r.status_code == 200:
                elements = r.json()['elements']
                result = {}
                for e in elements:
                    if e['type'] == 'node':
                        tags = e.get('tags', {})
                        result[str(e['id'])] = {
                            'lat':  e['lat'],
                            'lon':  e['lon'],
                            # name tag exists on landmarks/named intersections
                            'name': tags.get('name') or tags.get('name:en') or None,
                        }
                print(f"     Got {len(result)}/{len(osm_ids)} nodes from {url}")
                named = sum(1 for v in result.values() if v['name'])
                print(f"     {named} nodes have a name tag, {len(result)-named} need reverse geocoding")
                return result
            else:
                print(f"     {url}: HTTP {r.status_code}")
        except Exception as e:
            print(f"     {url}: {e}")
        time.sleep(2)

    print("     All Overpass mirrors failed!")
    return {}


# ── Step 3: Reverse-geocode unnamed nodes via Nominatim ──────────────────────

def reverse_geocode(lat, lon):
    """Return nearest road/area name from Nominatim, or None on failure."""
    try:
        r = requests.get(NOMINATIM_URL, params={
            'lat': lat, 'lon': lon,
            'format': 'json',
            'zoom': 17,          # street level
            'addressdetails': 1,
        }, headers=HEADERS, timeout=10)
        if r.status_code == 200:
            data = r.json()
            addr = data.get('address', {})
            # Build a short label: prefer road name, fall back to suburb/neighbourhood
            road   = addr.get('road') or addr.get('pedestrian') or addr.get('path')
            area   = addr.get('suburb') or addr.get('neighbourhood') or addr.get('city_district')
            if road and area:
                return f"{road}, {area}"
            elif road:
                return road
            elif area:
                return area
            else:
                return data.get('display_name', '').split(',')[0]
    except Exception as e:
        print(f"     Nominatim error for ({lat},{lon}): {e}")
    return None


def fill_names(osm_ids, raw_coords):
    """For nodes without a name tag, reverse-geocode to get a street label."""
    unnamed = [(i, osm_id) for i, osm_id in enumerate(osm_ids)
               if str(osm_id) in raw_coords and not raw_coords[str(osm_id)]['name']]

    if not unnamed:
        print("\n[3/3] All nodes already have names — skipping reverse geocoding.")
        return

    print(f"\n[3/3] Reverse-geocoding {len(unnamed)} unnamed nodes via Nominatim...")
    print(f"     (1 request/sec to respect rate limit — ~{len(unnamed)}s)")

    for i, (internal_id, osm_id) in enumerate(unnamed):
        node = raw_coords[str(osm_id)]
        name = reverse_geocode(node['lat'], node['lon'])
        if name:
            raw_coords[str(osm_id)]['name'] = name
        # Nominatim rate limit: max 1 req/sec
        time.sleep(1.1)
        if (i + 1) % 10 == 0:
            print(f"     {i+1}/{len(unnamed)} done...")

    print(f"     Reverse geocoding complete.")


# ── Assemble final JSON ───────────────────────────────────────────────────────

def assemble(osm_ids, raw_coords):
    result = {}
    no_data = []

    for internal_id, osm_id in enumerate(osm_ids):
        node = raw_coords.get(str(osm_id))
        if node:
            label = node.get('name') or f"Node {internal_id}"
            result[str(internal_id)] = {
                'lat':   node['lat'],
                'lon':   node['lon'],
                'osm':   osm_id,
                'name':  node.get('name') or '',   # raw OSM name tag (may be empty)
                'label': label,                     # what the frontend shows
            }
        else:
            no_data.append(osm_id)

    if no_data:
        print(f"     WARNING: {len(no_data)} nodes had no coordinate data at all.")

    # Add encoding='utf-8' here
    # Use the COORDS_OUT variable defined at the top of your script
    with open(COORDS_OUT, 'w', encoding='utf-8') as f:
        json.dump(result, f, indent=2, ensure_ascii=False)

    named_count = sum(1 for v in result.values() if v['name'])
    print(f"\n     Saved {len(result)} nodes to {COORDS_OUT}")
    print(f"     {named_count} nodes have real names, {len(result)-named_count} use 'Node N' fallback")
    return result


# ── Main ──────────────────────────────────────────────────────────────────────

if __name__ == '__main__':
    if not os.path.exists(GRAPH_FILE):
        print(f"ERROR: {GRAPH_FILE} not found.")
        print("Run this script from the DS2-Project/ root directory.")
        exit(1)

    # Step 1 — local
    osm_ids = build_cluster()

    # Step 2 — Overpass (lat/lon + name tags)
    raw_coords = fetch_overpass(osm_ids)
    if not raw_coords:
        print("\nFailed to fetch coordinates. Check your internet and try again.")
        print("cluster_graph.txt was written successfully.")
        exit(1)

    # Step 3 — Nominatim reverse geocode for unnamed nodes
    fill_names(osm_ids, raw_coords)

    # Assemble + save
    assemble(osm_ids, raw_coords)

    print("\n✓ Setup complete! Now run:")
    print("  g++ -std=c++17 src/main.cpp src/Graph.cpp src/FibHeap.cpp src/BinaryHeap.cpp -Isrc -O2 -o pathfinder.exe")
    print("  python app.py")