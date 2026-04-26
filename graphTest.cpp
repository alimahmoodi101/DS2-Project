#include "Graph.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>


// THIS TESTFILE IS TO TEST DIJIKSTRA

//helper functiosnss
static constexpr double INF = 1e18;
static constexpr double EPS = 1e-9;

void section(const char* title) {
    std::cout << "\n----------------------------------------\n";
    std::cout << "  " << title << "\n";
    std::cout << "------------------------------------------\n";
}

bool nearly_equal(double a, double b) {
    if (a == b) return true;
    if (a > 1e17 && b > 1e17) return true;   // both "infinite"
    return std::abs(a - b) < EPS;
}

// Verify both results match each other AND match expected values.
// expected[v] = -1  means "don't check this node"
void verify(const DijkstraResult& bin,
            const DijkstraResult& fib,
            const std::vector<double>& expected,
            const char* test_name)
{
    bool ok = true;
    int n = (int)bin.dist.size();

    for (int v = 0; v < n; ++v) {
        // Binary vs Fib must agree
        if (!nearly_equal(bin.dist[v], fib.dist[v])) {
            std::cout << "  MISMATCH at v=" << v
                      << "  binary=" << bin.dist[v]
                      << "  fib="    << fib.dist[v] << "\n";
            ok = false;
        }
        // vs expected (if provided)
        if (v < (int)expected.size() && expected[v] >= 0) {
            if (!nearly_equal(bin.dist[v], expected[v])) {
                std::cout << "  WRONG DISTANCE at v=" << v
                          << "  got=" << bin.dist[v]
                          << "  expected=" << expected[v] << "\n";
                ok = false;
            }
        }
    }

    if (ok) std::cout << "  [PASS] " << test_name << "\n";
    else    std::cout << "  [FAIL] " << test_name << "\n";

    assert(ok);
}

void print_result(const char* label, const DijkstraResult& r, int n) {
    std::cout << "  " << label << ":\n";
    for (int v = 0; v < n; ++v) {
        std::cout << "    dist[" << v << "] = ";
        if (r.dist[v] > 1e17) std::cout << "INF";
        else                  std::cout << r.dist[v];
        std::cout << "  (prev=" << r.prev[v] << ")\n";
    }
    std::cout << "    decrease_key calls = " << r.decrease_key_count << "\n";
    std::cout << "    time = " << r.time_ms << " ms\n";
}

void print_path(const DijkstraResult& r, int src, int dst) {
    std::vector<int> p = r.path(dst);
    std::cout << "    path " << src << "->" << dst << ": ";
    if (p.empty()) { std::cout << "UNREACHABLE\n"; return; }
    for (int i = 0; i < (int)p.size(); ++i) {
        if (i) std::cout << " -> ";
        std::cout << p[i];
    }
    std::cout << "  (dist=" << r.dist[dst] << ")\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// TEST 1 — Linear chain: 0->1->2->3->4
// Expected: d[0]=0, d[1]=1, d[2]=3, d[3]=6, d[4]=10
// ─────────────────────────────────────────────────────────────────────────────
void test_linear_chain() {
    section("TEST 1 - Linear chain (directed)");

    Graph g(5);
    g.add_edge(0, 1, 1);
    g.add_edge(1, 2, 2);
    g.add_edge(2, 3, 3);
    g.add_edge(3, 4, 4);

    g.print_graph();

    auto bin = g.dijkstra_binary(0);
    auto fib = g.dijkstra_fib(0);

    print_result("Binary Heap", bin, 5);
    print_result("Fib Heap   ", fib, 5);

    verify(bin, fib, {0, 1, 3, 6, 10}, "Linear chain distances");

    // Path check
    std::cout << "  Paths (binary):\n";
    for (int v = 1; v <= 4; ++v) print_path(bin, 0, v);
}

// ─────────────────────────────────────────────────────────────────────────────
// TEST 2 — Classic textbook graph (CLRS Fig 24.6 style, undirected)
//   Nodes: 0..4
//   Edges: 0-1(10), 0-3(5), 1-2(1), 1-3(2), 2-4(4), 3-1(3), 3-2(9), 3-4(2), 4-2(6), 4-0(7)
//   Source: 0
//   Expected: d[0]=0, d[1]=8, d[2]=9, d[3]=5, d[4]=7
// ─────────────────────────────────────────────────────────────────────────────
void test_clrs_graph() {
    section("TEST 2 - CLRS-style graph (directed, known answer)");

    Graph g(5);
    g.add_edge(0, 1, 10);
    g.add_edge(0, 3, 5);
    g.add_edge(1, 2, 1);
    g.add_edge(1, 3, 2);
    g.add_edge(2, 4, 4);
    g.add_edge(3, 1, 3);
    g.add_edge(3, 2, 9);
    g.add_edge(3, 4, 2);
    g.add_edge(4, 2, 6);
    g.add_edge(4, 0, 7);

    g.print_graph();

    auto bin = g.dijkstra_binary(0);
    auto fib = g.dijkstra_fib(0);

    print_result("Binary Heap", bin, 5);
    print_result("Fib Heap   ", fib, 5);

    verify(bin, fib, {0, 8, 9, 5, 7}, "CLRS graph distances");

    std::cout << "  Paths (binary):\n";
    for (int v = 1; v <= 4; ++v) print_path(bin, 0, v);
}

// ─────────────────────────────────────────────────────────────────────────────
// TEST 3 — Unreachable nodes
//   Two disconnected components: {0,1,2} and {3,4}
// ─────────────────────────────────────────────────────────────────────────────
void test_unreachable() {
    section("TEST 3 - Disconnected graph (unreachable nodes)");

    Graph g(5);
    g.add_edge(0, 1, 1);
    g.add_edge(1, 2, 1);
    // nodes 3 and 4 are completely isolated

    auto bin = g.dijkstra_binary(0);
    auto fib = g.dijkstra_fib(0);

    print_result("Binary Heap", bin, 5);
    print_result("Fib Heap   ", fib, 5);

    // d[3] and d[4] should be INF
    verify(bin, fib, {0, 1, 2, -1, -1}, "Reachable nodes correct");
    assert(bin.dist[3] > 1e17 && "d[3] should be INF");
    assert(bin.dist[4] > 1e17 && "d[4] should be INF");
    assert(fib.dist[3] > 1e17 && "d[3] should be INF (fib)");

    std::cout << "  d[3]=INF verified, d[4]=INF verified\n";
    std::cout << "  [PASS] Disconnected graph\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// TEST 4 — Multiple shortest path relaxations (many decrease-key calls)
//   Dense graph with shortcuts: source=0
// ─────────────────────────────────────────────────────────────────────────────
void test_multiple_relaxations() {
    section("TEST 4 - Multiple relaxations (high decrease-key count)");

    // 6-node graph where shortest paths require several updates
    //        1
    //      /   \
    //    1       5
    //  0           4 ─(1)─ 5
    //    4       2
    //      \   /
    //        3
    // Plus a shortcut 0->5 with weight 100 (should be beaten)
    Graph g(6);
    g.add_edge(0, 1, 1);
    g.add_edge(0, 3, 4);
    g.add_edge(0, 5, 100);  // expensive direct path, will be relaxed
    g.add_edge(1, 4, 5);
    g.add_edge(1, 2, 3);
    g.add_edge(2, 4, 1);
    g.add_edge(3, 2, 1);
    g.add_edge(4, 5, 1);

    // Expected shortest paths from 0:
    // d[1] = 1   (0->1)
    // d[2] = 4   (0->1->2)  or  (0->3->2 = 4+1=5), so 0->1->2=4
    // d[3] = 4   (0->3)
    // d[4] = 5   (0->1->2->4 = 1+3+1=5)  or  (0->1->4=1+5=6), so 5
    // d[5] = 6   (0->1->2->4->5 = 6)  beats 100
    auto bin = g.dijkstra_binary(0);
    auto fib = g.dijkstra_fib(0);

    print_result("Binary Heap", bin, 6);
    print_result("Fib Heap   ", fib, 6);

    verify(bin, fib, {0, 1, 4, 4, 5, 6}, "Multiple relaxations distances");

    std::cout << "  Paths (fib):\n";
    for (int v = 1; v <= 5; ++v) print_path(fib, 0, v);
}

// ─────────────────────────────────────────────────────────────────────────────
// TEST 5 — Single node graph
// ─────────────────────────────────────────────────────────────────────────────
void test_single_node() {
    section("TEST 5 - Single node graph");

    Graph g(1);
    auto bin = g.dijkstra_binary(0);
    auto fib = g.dijkstra_fib(0);

    assert(bin.dist[0] == 0.0);
    assert(fib.dist[0] == 0.0);
    std::cout << "  d[0]=0 for both. [PASS]\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// TEST 6 — Undirected grid graph (3x3), source = top-left (0)
//   Nodes laid out as:
//     0 ─ 1 ─ 2
//     |   |   |
//     3 ─ 4 ─ 5
//     |   |   |
//     6 ─ 7 ─ 8
//   All edge weights = 1
//   Expected distances from 0: Manhattan distance to each node
// ─────────────────────────────────────────────────────────────────────────────
void test_grid_graph() {
    section("TEST 6 - Undirected 3x3 grid (all weights=1)");

    Graph g(9);
    // Horizontal edges
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 2; ++col)
            g.add_undirected_edge(row*3 + col, row*3 + col + 1, 1.0);
    // Vertical edges
    for (int row = 0; row < 2; ++row)
        for (int col = 0; col < 3; ++col)
            g.add_undirected_edge(row*3 + col, (row+1)*3 + col, 1.0);

    g.print_graph();

    auto bin = g.dijkstra_binary(0);
    auto fib = g.dijkstra_fib(0);

    print_result("Binary Heap", bin, 9);
    print_result("Fib Heap   ", fib, 9);

    // Manhattan distances from (0,0):
    // node 0=(0,0)->0, 1=(0,1)->1, 2=(0,2)->2
    // node 3=(1,0)->1, 4=(1,1)->2, 5=(1,2)->3
    // node 6=(2,0)->2, 7=(2,1)->3, 8=(2,2)->4
    verify(bin, fib, {0,1,2,1,2,3,2,3,4}, "3x3 grid distances");

    std::cout << "  Path 0->8 (binary): ";
    auto p = bin.path(8);
    for (int i = 0; i < (int)p.size(); ++i) { if(i) std::cout<<"->"; std::cout<<p[i]; }
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// TEST 7 — Decrease-key count comparison
//   Both variants must report the SAME number of decrease-key operations
//   (since they process identical graph with identical relaxation order)
// ─────────────────────────────────────────────────────────────────────────────
void test_decrease_key_count() {
    section("TEST 7 - decrease-key counts match between both variants");

    Graph g(6);
    g.add_edge(0, 1, 2); g.add_edge(0, 2, 4);
    g.add_edge(1, 2, 1); g.add_edge(1, 3, 7);
    g.add_edge(2, 4, 3); g.add_edge(3, 5, 1);
    g.add_edge(4, 3, 2); g.add_edge(4, 5, 5);

    auto bin = g.dijkstra_binary(0);
    auto fib = g.dijkstra_fib(0);

    std::cout << "  Binary decrease_key calls : " << bin.decrease_key_count << "\n";
    std::cout << "  Fib    decrease_key calls : " << fib.decrease_key_count << "\n";

    assert(bin.decrease_key_count == fib.decrease_key_count
           && "decrease-key counts must be identical for same graph");

    verify(bin, fib, {}, "decrease-key count parity");
}



int main() {
    std::cout << "--------------------------------------------------------------------\n";
    std::cout << "|          Dijkstra Test Suite (Binary Heap vs Fib Heap)            |\n";
    std::cout << "--------------------------------------------------------------------\n";

    test_linear_chain();
    test_clrs_graph();
    test_unreachable();
    test_multiple_relaxations();
    test_single_node();
    test_grid_graph();
    test_decrease_key_count();

    std::cout << "\n--------------------------------------------------------------------\n";
    std::cout << "|                    All Dijkstra tests PASSED!                      |\n";
    std::cout << "|           Both implementations produce identical results           |\n";
    std::cout << "--------------------------------------------------------------------\n";
    return 0;
}