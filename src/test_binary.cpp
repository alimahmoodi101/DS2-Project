#include "BinaryHeap.h"
#include <iostream>
#include <cassert>

// ── helpers ──────────────────────────────────────────────────────────────────
void section(const char* title) {
    std::cout << "\n══════════════════════════════════════════\n";
    std::cout << "  " << title << "\n";
    std::cout << "══════════════════════════════════════════\n";
}

// ── tests ────────────────────────────────────────────────────────────────────

// Test 1: Basic min-order extraction
void test_basic_order() {
    section("TEST 1 – Basic min-order extraction");

    BinaryHeap h(6);
    h.insert(0, 10.0);
    h.insert(1,  3.0);
    h.insert(2,  7.0);
    h.insert(3,  1.0);
    h.insert(4,  5.0);

    std::cout << "Heap after 5 inserts:\n";
    h.print();

    std::cout << "\nExtracting in order (expect 1,3,5,7,10):\n";
    double prev = -1.0;
    while (!h.empty()) {
        BHNode n = h.extract_min();
        std::cout << "  extracted id=" << n.id << "  key=" << n.key << "\n";
        assert(n.key >= prev && "ERROR: extraction not in non-decreasing order!");
        prev = n.key;
    }
    std::cout << "  [PASS]\n";
}

// Test 2: decrease_key (lazy deletion path)
void test_decrease_key() {
    section("TEST 2 – decrease_key (lazy deletion)");

    BinaryHeap h(5);
    h.insert(0, 100.0);
    h.insert(1,  50.0);
    h.insert(2,  80.0);
    h.insert(3,  30.0);

    std::cout << "Before decrease_key:\n";
    h.print();

    // Decrease id=2 from 80 → 10 (should now be the minimum)
    h.decrease_key(2, 10.0);
    std::cout << "\nAfter decrease_key(id=2, key=10):\n";
    h.print();

    BHNode top = h.extract_min();
    std::cout << "\nextract_min → id=" << top.id << "  key=" << top.key << "\n";
    assert(top.id == 2 && top.key == 10.0 && "ERROR: expected id=2, key=10");

    // Also decrease id=0 from 100 → 5
    h.decrease_key(0, 5.0);
    top = h.extract_min();
    std::cout << "extract_min → id=" << top.id << "  key=" << top.key << "\n";
    assert(top.id == 0 && top.key == 5.0 && "ERROR: expected id=0, key=5");

    std::cout << "  [PASS]\n";
}

// Test 3: duplicate keys
void test_duplicate_keys() {
    section("TEST 3 – Duplicate keys");

    BinaryHeap h(5);
    h.insert(0, 5.0);
    h.insert(1, 5.0);
    h.insert(2, 5.0);

    std::cout << "Three nodes all with key=5.0:\n";
    h.print();

    double prev = -1.0;
    while (!h.empty()) {
        BHNode n = h.extract_min();
        std::cout << "  extracted id=" << n.id << "  key=" << n.key << "\n";
        assert(n.key >= prev);
        prev = n.key;
    }
    std::cout << "  [PASS]\n";
}

// Test 4: single element
void test_single_element() {
    section("TEST 4 – Single element");

    BinaryHeap h(2);
    h.insert(0, 42.0);

    assert(!h.empty());
    BHNode n = h.extract_min();
    assert(n.id == 0 && n.key == 42.0);
    assert(h.empty());

    std::cout << "  Inserted id=0 key=42, extracted correctly.\n";
    std::cout << "  [PASS]\n";
}

// Test 5: Simulated Dijkstra-style relaxation sequence
void test_dijkstra_simulation() {
    section("TEST 5 – Dijkstra-style relaxation simulation");

    // Small graph:  0--(1)-->1--(2)-->2--(1)-->3
    //                \--(10)------------>3
    // Shortest paths from 0: d[0]=0, d[1]=1, d[2]=3, d[3]=4

    const int N = 4;
    BinaryHeap h(N);

    // Source = 0
    h.insert(0, 0.0);
    h.insert(1, 1e18);
    h.insert(2, 1e18);
    h.insert(3, 1e18);

    std::cout << "Initial heap (source=0 at dist 0, rest at INF):\n";
    h.print();

    // Relax from 0
    h.decrease_key(1, 0.0 + 1.0);   // edge 0→1 weight 1
    h.decrease_key(3, 0.0 + 10.0);  // edge 0→3 weight 10

    // Extract source 0 (dist 0)
    BHNode src = h.extract_min();
    std::cout << "\nSettled source: id=" << src.id << " dist=" << src.key << "\n";
    assert(src.id == 0 && src.key == 0.0);

    // Extract 1 (dist 1), relax its neighbours
    BHNode u = h.extract_min();
    std::cout << "Settled: id=" << u.id << " dist=" << u.key << "\n";
    assert(u.id == 1 && u.key == 1.0);
    h.decrease_key(2, 1.0 + 2.0);   // edge 1→2 weight 2

    // Extract 2 (dist 3)
    u = h.extract_min();
    std::cout << "Settled: id=" << u.id << " dist=" << u.key << "\n";
    assert(u.id == 2 && u.key == 3.0);
    h.decrease_key(3, 3.0 + 1.0);   // edge 2→3 weight 1, improves 10→4

    // Extract 3 (dist 4, after relaxation)
    u = h.extract_min();
    std::cout << "Settled: id=" << u.id << " dist=" << u.key << "\n";
    assert(u.id == 3 && u.key == 4.0);

    std::cout << "  Shortest paths verified: d[1]=1, d[2]=3, d[3]=4\n";
    std::cout << "  [PASS]\n";
}

// ── main ─────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║     BinaryHeap Test Suite                ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";

    test_basic_order();
    test_decrease_key();
    test_duplicate_keys();
    test_single_element();
    test_dijkstra_simulation();

    std::cout << "\n══════════════════════════════════════════\n";
    std::cout << "  All BinaryHeap tests PASSED\n";
    std::cout << "══════════════════════════════════════════\n";
    return 0;
}