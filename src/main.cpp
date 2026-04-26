#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <sstream>
#include <tuple>
#include <algorithm>
#include "Graph.h"
using namespace std;

Graph* loadGraph(const string& filename, int& out_nodes) {
    ifstream f(filename);
    if (!f.is_open()) {
        cerr << "{\"error\": \"Cannot open " << filename << "\"}" << endl;
        return nullptr;
    }

    // First pass: find max node id
    vector<tuple<int,int,double>> edges;
    int maxId = 0;
    int u, v; double w;
    while (f >> u >> v >> w) {
        edges.push_back({u, v, w});
        maxId = max(maxId, max(u, v));
    }

    out_nodes = maxId + 1;
    Graph* g = new Graph(out_nodes);
    for (auto& [a, b, wt] : edges)
        g->add_undirected_edge(a, b, wt);

    return g;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "{\"error\": \"Usage: ./pathfinder <start> <end>\"}" << endl;
        return 1;
    }

    int start_node = stoi(argv[1]);
    int end_node   = stoi(argv[2]);

    int total_nodes = 0;
    Graph* g = loadGraph("src/cluster_graph.txt", total_nodes);
    if (!g) return 1;

    if (start_node < 0 || start_node >= total_nodes ||
        end_node   < 0 || end_node   >= total_nodes) {
        cout << "{\"error\": \"Node IDs must be between 0 and " << total_nodes-1 << "\"}" << endl;
        delete g;
        return 1;
    }

    DijkstraResult bin = g->dijkstra_binary(start_node);
    DijkstraResult fib = g->dijkstra_fib(start_node);
    delete g;

    double dist = bin.dist[end_node];
    bool reachable = dist < numeric_limits<double>::infinity();

    string path_str = "[";
    if (reachable) {
        vector<int> path = bin.path(end_node);
        for (int i = 0; i < (int)path.size(); i++) {
            if (i) path_str += ",";
            path_str += to_string(path[i]);
        }
    }
    path_str += "]";

    cout << "{"
         << "\"reachable\":"          << (reachable ? "true" : "false") << ","
         << "\"distance_meters\":"    << (reachable ? dist : 0.0)       << ","
         << "\"path\":"               << path_str                        << ","
         << "\"binary_time_ms\":"     << bin.time_ms                     << ","
         << "\"binary_decrease_keys\":" << bin.decrease_key_count        << ","
         << "\"fib_time_ms\":"        << fib.time_ms                     << ","
         << "\"fib_decrease_keys\":"  << fib.decrease_key_count
         << "}" << endl;
    return 0;
}