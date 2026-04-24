import requests
import math

def calculate_distance(lat1, lon1, lat2, lon2):
    #The Haversine formula is used to calculate the shortest distance between two points on a sphere, given their latitudes and longitudes
    R = 6371000  
    phi1 = math.radians(lat1)
    phi2 = math.radians(lat2)
    delta_phi = math.radians(lat2 - lat1)
    delta_lambda = math.radians(lon2 - lon1)
    a = math.sin(delta_phi/2.0)**2 + math.cos(phi1) * math.cos(phi2) * math.sin(delta_lambda/2.0)**2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))                                                  
    
    return R * c 

def fetch_and_parse_osm():
    url = "http://overpass-api.de/api/interpreter"
    
    query = """
    [out:json][timeout:90];
    (
      way["highway"]["highway"!~"pedestrian|footway|path|steps"](24.85, 67.00, 24.95, 67.10);
    );
    out body;
    >;
    out skel qt;
    """

    headers = {
        'User-Agent': 'CS201-Pathfinding-Project/1.0 (student-project)'
    }

    print("Ordering data from the Overpass API")
    
    response = requests.post(url, data={'data': query}, headers=headers)

    if response.status_code != 200:
        print(f"Error! The API returned status code: {response.status_code}")
        return

    data = response.json()
    print("Parsing into graph format...")

    nodes = {} 
    ways = []  

    for element in data['elements']:
        if element['type'] == 'node':
            nodes[element['id']] = (element['lat'], element['lon'])
        elif element['type'] == 'way':
            if 'nodes' in element:
                ways.append(element['nodes'])

    output_file = "karachi_graph.txt"
    edge_count = 0

    with open(output_file, 'w') as f:
        for way in ways:
            for i in range(len(way) - 1):
                nodeA = way[i]
                nodeB = way[i+1]
                if nodeA in nodes and nodeB in nodes:
                    lat1, lon1 = nodes[nodeA]
                    lat2, lon2 = nodes[nodeB]

                    weight = calculate_distance(lat1, lon1, lat2, lon2)
                    f.write(f"{nodeA} {nodeB} {weight:.2f}\n")
                    edge_count += 1

    print(f"Success! Wrote {edge_count} edges to {output_file}")

if __name__ == "__main__":
    fetch_and_parse_osm()