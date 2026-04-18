#pragma once
#include <vector>
#include <string>

// ── Edge ────────────────────────────────────────────────────────────────────
struct Edge {
    int    to;
    double weight;
};

// ── DijkstraResult ───────────────────────────────────────────────────────────
// Returned by both Dijkstra variants so results can be compared directly.
struct DijkstraResult {
    std::vector<double> dist;        // dist[v] = shortest distance from source to v
    std::vector<int>    prev;        // prev[v] = predecessor of v on shortest path
    long long           decrease_key_count = 0;  // total decrease-key operations
    double              time_ms = 0.0;            // wall-clock time in milliseconds

    // Reconstruct path from source to target (returns empty if unreachable)
    std::vector<int> path(int target) const;
};

// ── Graph ────────────────────────────────────────────────────────────────────
class Graph {
public:
    explicit Graph(int n);           // n = number of nodes (0-indexed)

    void add_edge(int u, int v, double w);   // directed edge u → v
    void add_undirected_edge(int u, int v, double w);  // both directions

    int num_nodes() const;
    int num_edges() const;

    const std::vector<Edge>& neighbours(int u) const;

    // ── Dijkstra variants ────────────────────────────────────────────────────
    // Both produce identical DijkstraResult; only the internal heap differs.
    DijkstraResult dijkstra_binary(int source) const;
    DijkstraResult dijkstra_fib   (int source) const;

    void print_graph() const;

private:
    int                              n_;
    int                              edge_count_;
    std::vector<std::vector<Edge>>   adj_;   // adjacency list
};