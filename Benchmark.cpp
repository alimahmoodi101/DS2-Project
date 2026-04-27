// benchmark.cpp
// ─────────────────────────────────────────────────────────────────────────────
//  Benchmarks Binary Heap vs Fibonacci Heap Dijkstra on any graph file.
//
//  IMPORTANT – the graph file must use *sequential* 0-based integer node IDs.
//  If you are loading karachi_graph.txt (raw OSM IDs like 2445564262) you must
//  first remap them.  Run the Python helper first:
//
//    python3 remap_ids.py src/karachi_graph.txt karachi_remapped.txt
//
//  Then pass the remapped file to this program:
//
//    ./benchmark karachi_remapped.txt 200
//    ./benchmark src/cluster_graph.txt 500
//
//  Usage: ./benchmark <graph_file> [num_trials=200]
// ─────────────────────────────────────────────────────────────────────────────
 
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <random>
#include <tuple>
#include <algorithm>
#include <iomanip>
#include <string>
#include "Graph.h"
 
// ── Graph loader ─────────────────────────────────────────────────────────────
// Reads "u v weight\n" lines.  Node IDs must be 0-based integers.
// Returns nullptr on failure.
Graph* load_graph(const std::string& filename, int& out_nodes, int& out_edges) {
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "ERROR: Cannot open '" << filename << "'\n";
        return nullptr;
    }
 
    std::vector<std::tuple<int,int,double>> raw;
    int maxId = 0, u, v; double w;
    while (f >> u >> v >> w) {
        raw.emplace_back(u, v, w);
        maxId = std::max(maxId, std::max(u, v));
    }
 
    out_nodes = maxId + 1;
    out_edges = (int)raw.size();
    Graph* g = new Graph(out_nodes);
    for (auto& [a, b, wt] : raw)
        g->add_undirected_edge(a, b, wt);
 
    return g;
}
 
// ── Bucket helpers ────────────────────────────────────────────────────────────
struct TrialResult {
    double binary_ms;
    double fib_ms;
    long long binary_dk;
    long long fib_dk;
    double dist;          // shortest path distance (from binary, for categorisation)
};
 
struct BucketStats {
    std::string label;
    std::vector<double> binary_times, fib_times;
    std::vector<long long> dk_counts;
    int count = 0;
 
    void add(double bt, double ft, long long dk) {
        binary_times.push_back(bt);
        fib_times.push_back(ft);
        dk_counts.push_back(dk);
        ++count;
    }
 
    double mean(const std::vector<double>& v) const {
        if (v.empty()) return 0;
        double s = 0; for (double x : v) s += x; return s / v.size();
    }
    double meanLL(const std::vector<long long>& v) const {
        if (v.empty()) return 0;
        double s = 0; for (long long x : v) s += x; return s / v.size();
    }
    double median(std::vector<double> v) const {
        if (v.empty()) return 0;
        std::sort(v.begin(), v.end());
        return v[v.size()/2];
    }
};
 
// ── Pretty print ─────────────────────────────────────────────────────────────
void print_separator(char c = '-', int width = 72) {
    std::cout << std::string(width, c) << '\n';
}
 
void print_bucket(const BucketStats& b) {
    if (b.count == 0) { std::cout << "  (no trials)\n"; return; }
 
    double bm = b.mean(b.binary_times);
    double fm = b.mean(b.fib_times);
    double bmed = b.median(b.binary_times);
    double fmed = b.median(b.fib_times);
    double ratio = (bm > 0) ? fm / bm : 0;
    double avg_dk = b.meanLL(b.dk_counts);
 
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  Trials          : " << b.count << "\n";
    std::cout << "  Avg decrease-key: " << (long long)avg_dk << "\n";
    std::cout << "  Binary  mean/med: " << bm  << " ms / " << bmed  << " ms\n";
    std::cout << "  FibHeap mean/med: " << fm  << " ms / " << fmed  << " ms\n";
    std::cout << "  Fib/Binary ratio: " << std::setprecision(2) << ratio
              << "x  (" << (ratio < 1.0 ? "Fib FASTER" : ratio > 1.0 ? "Binary FASTER" : "EQUAL") << ")\n";
}
 
// ── Main ─────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    std::string filename = "src/cluster_graph.txt";
    int trials = 200;
 
    if (argc >= 2) filename = argv[1];
    if (argc >= 3) trials = std::stoi(argv[2]);
 
    print_separator('=');
    std::cout << "  Dijkstra Benchmark: Binary Heap vs Fibonacci Heap\n";
    print_separator('=');
 
    int n_nodes = 0, n_edges = 0;
    Graph* g = load_graph(filename, n_nodes, n_edges);
    if (!g) return 1;
 
    // Graph stats
    double density = (n_nodes > 1)
        ? (double)n_edges / ((double)n_nodes * (n_nodes - 1))
        : 0.0;
 
    std::cout << "  File    : " << filename << "\n";
    std::cout << "  Nodes   : " << n_nodes  << "\n";
    std::cout << "  Edges   : " << n_edges  << " (undirected, stored as 2x directed)\n";
    std::cout << "  Density : " << std::scientific << std::setprecision(4) << density;
    std::cout << (density < 1e-3 ? "  [SPARSE — Fib advantage limited]"
                                 : "  [DENSE]") << "\n";
    std::cout << "  Trials  : " << trials << "\n";
    print_separator();
 
    // Random source node selection
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, n_nodes - 1);
 
    BucketStats overall, short_q, medium_q, long_q;
    overall.label = "ALL";
    short_q.label  = "SHORT  (dist < 1 km)";
    medium_q.label = "MEDIUM (1–10 km)";
    long_q.label   = "LONG   (> 10 km)";
 
    // Warm-up run (not counted)
    {
        int s = dis(gen);
        g->dijkstra_binary(s);
        g->dijkstra_fib(s);
    }
 
    std::cout << "  Running " << trials << " trials";
 
    for (int i = 0; i < trials; ++i) {
        if (i % (trials/10) == 0) { std::cout << '.'; std::cout.flush(); }
 
        int src = dis(gen);
 
        auto rb = g->dijkstra_binary(src);
        auto rf = g->dijkstra_fib(src);
 
        // Use a random reachable destination (find first settled non-INF node)
        // For fair comparison, the timing already covers full SSSP from src.
        double bt = rb.time_ms, ft = rf.time_ms;
        long long dk = rf.decrease_key_count;
 
        // Categorise by average finite distance
        double sum = 0; int cnt = 0;
        for (double d : rb.dist) if (d < 1e17) { sum += d; ++cnt; }
        double avg_dist = cnt ? sum / cnt : 0.0;
 
        overall.add(bt, ft, dk);
        if      (avg_dist < 1000.0)  short_q .add(bt, ft, dk);
        else if (avg_dist < 10000.0) medium_q.add(bt, ft, dk);
        else                         long_q  .add(bt, ft, dk);
    }
 
    std::cout << "\n";
    print_separator();
 
    std::cout << "  OVERALL RESULTS\n";
    print_separator();
    print_bucket(overall);
    print_separator();
 
    // Categorised
    for (BucketStats* b : {&short_q, &medium_q, &long_q}) {
        std::cout << "\n  BUCKET: " << b->label << "\n";
        print_separator('-', 40);
        print_bucket(*b);
    }
 
    print_separator('=');
    std::cout << "  INTERPRETATION\n";
    print_separator('=');
    {
        double bm = overall.mean(overall.binary_times);
        double fm = overall.mean(overall.fib_times);
        double ratio = (bm > 0) ? fm / bm : 0;
        double avg_dk = overall.meanLL(overall.dk_counts);
 
        std::cout << "  Graph density: " << std::scientific << density << "\n";
        std::cout << "  Avg decrease-key ops/query: " << (long long)avg_dk
                  << " out of " << n_nodes << " nodes\n";
        double dk_ratio = n_nodes ? avg_dk / n_nodes : 0;
        std::cout << "  Decrease-key / node ratio: " << std::fixed
                  << std::setprecision(3) << dk_ratio << "\n";
 
        if (ratio <= 1.0)
            std::cout << "\n  => Fibonacci Heap is FASTER (ratio=" << ratio << "x)\n";
        else {
            std::cout << "\n  => Binary Heap is faster (ratio=" << ratio << "x)\n";
            std::cout << "     This is expected for sparse graphs: Fib-heap has higher\n";
            std::cout << "     constant factors in cache performance and pointer chasing.\n";
            std::cout << "     The O(1) amortised decrease-key only pays off when\n";
            std::cout << "     dk_ops >> V*log(V), i.e. on very dense graphs.\n";
            std::cout << "     For this graph: V*log(V) = " << std::fixed << std::setprecision(0)
                      << (n_nodes * std::log2(n_nodes)) << "  vs  avg_dk = "
                      << (long long)avg_dk << "\n";
        }
    }
    print_separator('=');
 
    delete g;
    return 0;
}