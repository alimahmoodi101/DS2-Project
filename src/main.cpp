#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <limits>
#include "Graph.h"
using namespace std;

struct RawEdge {
    long long u_osm, v_osm;
    double weight;
};

Graph* loadGraphFromFile(const string& filename, 
                         unordered_map<long long, int>& osm_to_internal,
                         vector<long long>& internal_to_osm) {
    
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error: Could not open " << filename << "!\n";
        return nullptr;
    }

    vector<RawEdge> raw_edges;
    int current_internal_id = 0;
    long long u, v;
    double w;
    
    cout << "Loading map data from " << filename << "...\n";
    while (infile >> u >> v >> w) {
        raw_edges.push_back({u, v, w});
        
        if (osm_to_internal.find(u) == osm_to_internal.end()) {
            osm_to_internal[u] = current_internal_id++;
            internal_to_osm.push_back(u);
        }
        if (osm_to_internal.find(v) == osm_to_internal.end()) {
            osm_to_internal[v] = current_internal_id++;
            internal_to_osm.push_back(v);
        }
    }
    infile.close();

    int total_nodes = current_internal_id;
    cout << "Data loaded! Found " << total_nodes << " nodes and " 
         << raw_edges.size() << " edges.\n";

    Graph* city_graph = new Graph(total_nodes);
    for (const auto& edge : raw_edges) {
        int internal_u = osm_to_internal[edge.u_osm];
        int internal_v = osm_to_internal[edge.v_osm];
        city_graph->add_undirected_edge(internal_u, internal_v, edge.weight);
    }

    return city_graph;
}


void runPathfindingQuery(const Graph* city_graph, int start_node, int end_node) {
    cout << "\n--- Pathfinding Query Initialized ---\n";
    cout << "Calculating shortest path from Node " << start_node 
         << " to Node " << end_node << "...\n\n";

    // Standard Binary Heap
    DijkstraResult res_bin = city_graph->dijkstra_binary(start_node);
    
    // Custom Fibonacci Heap
    DijkstraResult res_fib = city_graph->dijkstra_fib(start_node);

    if (res_bin.dist[end_node] >= numeric_limits<double>::infinity()) {
        cout << "Result: No valid path exists between these two nodes.\n";
        return;
    }

    cout << "Shortest Distance: " << res_bin.dist[end_node] << " meters\n\n";
    
    cout << left << setw(15) << "Data Structure" 
         << setw(20) << "Execution Time (ms)" 
         << setw(25) << "Decrease-Key Operations" << "\n";
    
    cout << left << setw(15) << "Binary Heap" 
         << setw(20) << res_bin.time_ms 
         << setw(25) << res_bin.decrease_key_count << "\n";
              
    cout << left << setw(15) << "Fibonacci Heap" 
         << setw(20) << res_fib.time_ms 
         << setw(25) << res_fib.decrease_key_count << "\n";
}

int main() {
    unordered_map<long long, int> osm_to_internal;
    vector<long long> internal_to_osm; 

    Graph* city_graph = loadGraphFromFile("karachi_graph.txt", osm_to_internal, internal_to_osm);
    
    if (!city_graph) return 1;

    // Functionality 2 Call (Hardcoded for initial implementation checkpoint)
    runPathfindingQuery(city_graph, 0, 1000);
    
    // Testing another longer route
    runPathfindingQuery(city_graph, 50, 25000);

    delete city_graph;
    return 0;
}