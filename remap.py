"""
remap_ids.py  –  Convert a graph file with arbitrary node IDs (e.g. raw OSM
                 IDs like 2445564262) to sequential 0-based integers that
                 Graph.h can load without allocating billions of array slots.

Usage:
    python3 remap_ids.py src/karachi_graph.txt karachi_remapped.txt

Output:
    <output_file>   – same edge list with remapped integer IDs
    <output_file>.map.txt – mapping: original_id -> new_id  (one per line)
"""

import sys

def remap(infile: str, outfile: str):
    id_map = {}
    counter = 0
    edges = []

    with open(infile) as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) < 3:
                continue
            a, b, w = parts[0], parts[1], parts[2]
            if a not in id_map:
                id_map[a] = counter
                counter += 1
            if b not in id_map:
                id_map[b] = counter
                counter += 1
            edges.append((id_map[a], id_map[b], w))

    with open(outfile, 'w') as f:
        for a, b, w in edges:
            f.write(f"{a} {b} {w}\n")

    mapfile = outfile + ".map.txt"
    with open(mapfile, 'w') as f:
        for orig, new_id in id_map.items():
            f.write(f"{orig} -> {new_id}\n")

    print(f"Remapped {len(id_map)} nodes, {len(edges)} edges")
    print(f"  Output graph : {outfile}")
    print(f"  ID map       : {mapfile}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 remap_ids.py <input_graph> <output_graph>")
        sys.exit(1)
    remap(sys.argv[1], sys.argv[2])