#include "Graph.h"
#include "FibHeap.h"

#include <iostream>
#include <limits>
#include <chrono>
#include <algorithm>
#include <queue>        // std::priority_queue

static constexpr double INF = std::numeric_limits<double>::infinity();

// ── DijkstraResult ───────────────────────────────────────────────────────────
std::vector<int> DijkstraResult::path(int target) const {
    std::vector<int> p;
    if (dist[target] >= INF) return p;   // unreachable
    for (int v = target; v != -1; v = prev[v])
        p.push_back(v);
    std::reverse(p.begin(), p.end());
    return p;
}

// ── Graph ────────────────────────────────────────────────────────────────────
Graph::Graph(int n) : n_(n), edge_count_(0), adj_(n) {}

void Graph::add_edge(int u, int v, double w) {
    adj_[u].push_back({v, w});
    ++edge_count_;
}

void Graph::add_undirected_edge(int u, int v, double w) {
    add_edge(u, v, w);
    add_edge(v, u, w);
}

int Graph::num_nodes() const { return n_; }
int Graph::num_edges() const { return edge_count_; }

const std::vector<Edge>& Graph::neighbours(int u) const { return adj_[u]; }

void Graph::print_graph() const {
    std::cout << "[Graph] nodes=" << n_ << "  edges=" << edge_count_ << "\n";
    for (int u = 0; u < n_; ++u) {
        std::cout << "  " << u << " -> ";
        for (const Edge& e : adj_[u])
            std::cout << e.to << "(w=" << e.weight << ")  ";
        std::cout << "\n";
    }
}

// ── Dijkstra using std::priority_queue (STL binary heap, lazy deletion) ──────
//
//  This is exactly what the proposal specifies: "C++ standard library's
//  std::priority_queue (binary heap)".
//
//  Lazy deletion pattern:
//    - push {dist, node} whenever we find a shorter path
//    - on pop, skip the entry if we have already settled that node
//    - no in-place update needed, stale entries are just ignored
//
DijkstraResult Graph::dijkstra_binary(int source) const {
    auto t_start = std::chrono::high_resolution_clock::now();

    DijkstraResult res;
    res.dist.assign(n_, INF);
    res.prev.assign(n_, -1);
    res.decrease_key_count = 0;

    // STL min-heap: pair<distance, node_id>
    // priority_queue is a max-heap by default, so we negate or use greater<>
    using PQEntry = std::pair<double, int>;
    std::priority_queue<PQEntry,
                        std::vector<PQEntry>,
                        std::greater<PQEntry>> pq;

    res.dist[source] = 0.0;
    pq.push({0.0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        // Lazy deletion: skip if this entry is stale
        if (d > res.dist[u] + 1e-9) continue;

        for (const Edge& e : adj_[u]) {
            double new_dist = d + e.weight;
            if (new_dist < res.dist[e.to]) {
                res.dist[e.to] = new_dist;
                res.prev[e.to] = u;
                pq.push({new_dist, e.to});   // lazy: push new entry, old one goes stale
                ++res.decrease_key_count;
            }
        }
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    res.time_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    return res;
}

// ── Dijkstra using custom Fibonacci Heap ─────────────────────────────────────
//
//  Uses true O(1) amortised decrease-key via our hand-built FibHeap.
//  A handle (FibNode*) per node is kept so decrease_key needs no search.
//
DijkstraResult Graph::dijkstra_fib(int source) const {
    auto t_start = std::chrono::high_resolution_clock::now();

    DijkstraResult res;
    res.dist.assign(n_, INF);
    res.prev.assign(n_, -1);
    res.decrease_key_count = 0;

    FibHeap pq;

    // Insert every node up front so decrease_key can be called on any of them
    std::vector<FibNode*> handles(n_, nullptr);
    std::vector<bool>     settled(n_, false);

    res.dist[source] = 0.0;
    for (int v = 0; v < n_; ++v)
        handles[v] = pq.insert(v, (v == source) ? 0.0 : INF);

    while (!pq.empty()) {
        FibNode* top = pq.extract_min();
        int    u = top->id;
        double d = top->key;
        delete top;
        handles[u] = nullptr;

        if (d >= INF) break;   // remaining nodes are unreachable
        settled[u] = true;

        for (const Edge& e : adj_[u]) {
            int v = e.to;
            if (settled[v]) continue;

            double new_dist = d + e.weight;
            if (new_dist < res.dist[v]) {
                res.dist[v] = new_dist;
                res.prev[v] = u;
                pq.decrease_key(handles[v], new_dist);   // true O(1) decrease-key
                ++res.decrease_key_count;
            }
        }
    }

    // Drain unreachable nodes still sitting in the heap
    while (!pq.empty()) delete pq.extract_min();

    auto t_end = std::chrono::high_resolution_clock::now();
    res.time_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    return res;
}