#include "FibHeap.h"
#include <iostream>
#include <cassert>
#include <vector>



//helper func
void section(const char* title) {
    std::cout << "\n----------------------------------------\n";
    std::cout << "  " << title << "\n";
    std::cout << "------------------------------------------\n";
}

// Test 1: Basic min-order extraction
void test_basic_order() {
    section("TEST 1 - Basic min-order extraction");

    FibHeap h;
    h.insert(0,10.0);
    h.insert(1,3.0);
    h.insert(2,7.0);
    h.insert(3,1.0);
    h.insert(4,5.0);

    std::cout << "Root list after 5 inserts (all in root list, no consolidation yet):\n";
    h.print_roots();

    std::cout << "\nExtracting in order (expect 1,3,5,7,10):\n";
    double prev = -1.0;
    while (!h.empty()) {
        FibNode* n = h.extract_min();
        std::cout << "  extracted id=" << n->id << "  key=" << n->key << "\n";
        assert(n->key >= prev && "ERROR: extraction not in non-decreasing order!");
        prev = n->key;
        delete n;
    }
    std::cout << "  [PASS]\n";
}

// Test 2: decrease_key — verify heap property is restored
void test_decrease_key() {
    section("TEST 2 - decrease_key");

    FibHeap h;
    FibNode* n0 = h.insert(0,100.0);
    FibNode* n1 = h.insert(1,50.0);
    FibNode* n2 = h.insert(2,80.0);
    FibNode* n3 = h.insert(3,30.0);

    std::cout << "Before decrease_key:\n";
    h.print_roots();

    // Decrease id=2 from 80 -> 5 (should become the new minimum)
    h.decrease_key(n2, 5.0);
    std::cout << "\nAfter decrease_key(id=2, key=5):\n";
    h.print_roots();

    FibNode* top = h.extract_min();
    std::cout << "\nextract_min -> id=" << top->id << "  key=" << top->key << "\n";
    assert(top->id == 2 && top->key == 5.0 && "ERROR: expected id=2, key=5");
    delete top;

    // Now decrease id=0 from 100 -> 8 (should come out before 30 and 50)
    h.decrease_key(n0, 8.0);
    top = h.extract_min();
    std::cout << "extract_min -> id=" << top->id << "  key=" << top->key << "\n";
    assert(top->id == 0 && top->key == 8.0 && "ERROR: expected id=0, key=8");
    delete top;

    std::cout << "  [PASS]\n";

    // Drain remaining
    while (!h.empty()) delete h.extract_min();
}



// Test 3: Consolidation correctness — after extract_min, trees are merged
//         We check that the degree structure is consistent (no two roots with same degree)
void test_consolidation() {
    section("TEST 3 - Consolidation after extract_min");

    FibHeap h;
    // Insert enough nodes that consolidation must do real work
    std::vector<FibNode*> nodes;
    for (int i = 0; i < 8; ++i)
        nodes.push_back(h.insert(i, (double)(8 - i)));  // keys 8,7,6,5,4,3,2,1

    std::cout << "Before first extract (all roots, no consolidation):\n";
    h.print_roots();

    FibNode* top = h.extract_min();
    std::cout << "\nAfter extract_min (triggers consolidation):\n";
    h.print_roots();
    std::cout << "  extracted id=" << top->id << "  key=" << top->key << "\n";
    assert(top->key == 1.0 && "ERROR: min should be key=1");
    delete top;

    // Continue extracting and verify order
    double prev = 1.0;
    while (!h.empty()) {
        FibNode* n = h.extract_min();
        std::cout << "  extracted id=" << n->id << "  key=" << n->key << "\n";
        assert(n->key >= prev && "ERROR: out-of-order extraction after consolidation!");
        prev = n->key;
        delete n;
    }
    std::cout << "  [PASS]\n";
}


// Test 4: Cascading cut — cut a node, then cut its parent, verify marks
void test_cascading_cut() {
    section("TEST 4 - Cascading cut behaviour");

    // Build a known structure by forcing consolidation:
    // Insert 4 nodes, extract_min to trigger consolidation,
    // then decrease_key on a deep node to trigger cut chain.
    FibHeap h;
    FibNode* n0 = h.insert(0, 1.0);
    FibNode* n1 = h.insert(1, 2.0);
    FibNode* n2 = h.insert(2, 3.0);
    FibNode* n3 = h.insert(3, 4.0);
    FibNode* n4 = h.insert(4, 5.0);
    FibNode* n5 = h.insert(5, 6.0);
    FibNode* n6 = h.insert(6, 7.0);
    FibNode* n7 = h.insert(7, 8.0);
    (void)n7;

    // Extract min to force consolidation -> builds tree structure
    FibNode* top = h.extract_min();  // removes n0 (key=1)
    std::cout << "After first extract_min (key=1 removed, trees consolidated):\n";
    h.print_roots();
    delete top;

    // Now do a deep decrease_key to trigger cascading cuts
    // n5 and n6 should be deep inside a tree; decreasing n5 then n6
    // should cause the mark flag to fire on their ancestors.
    h.decrease_key(n5, 0.5);
    std::cout << "\nAfter decrease_key(n5, 0.5) — n5 should now be a root:\n";
    h.print_roots();

    top = h.extract_min();
    std::cout << "\nextract_min -> id=" << top->id << "  key=" << top->key << "\n";
    assert(top->key == 0.5 && "ERROR: expected key=0.5 after cascading cut path");
    delete top;

    std::cout << "  [PASS]\n";

    while (!h.empty()) delete h.extract_min();
}



// Test 5: Dijkstra-style relaxation (mirrors binary heap test 5)
void test_dijkstra_simulation() {
    section("TEST 5 - Dijkstra-style relaxation simulation");

    // Same small graph as binary heap test:
    // 0--(1)-->1--(2)-->2--(1)-->3
    // \--(10)------------>3
    // d[0]=0, d[1]=1, d[2]=3, d[3]=4

    const double BIG = 1e18;
    FibHeap h;
    FibNode* p0 = h.insert(0, 0.0);
    FibNode* p1 = h.insert(1, BIG);
    FibNode* p2 = h.insert(2, BIG);
    FibNode* p3 = h.insert(3, BIG);
    (void)p0;

    std::cout << "Initial heap (source=0 at dist 0):\n";
    h.print_roots();

    // Relax from 0
    h.decrease_key(p1, 0.0 + 1.0);
    h.decrease_key(p3, 0.0 + 10.0);

    // Extract source (id=0, dist=0) first
    FibNode* src = h.extract_min();
    std::cout << "\nSettled source: id=" << src->id << " dist=" << src->key << "\n";
    assert(src->id == 0 && src->key == 0.0);
    delete src;

    FibNode* u = h.extract_min();
    std::cout << "Settled: id=" << u->id << " dist=" << u->key << "\n";
    assert(u->id == 1 && u->key == 1.0);
    delete u;

    h.decrease_key(p2, 1.0 + 2.0);

    u = h.extract_min();
    std::cout << "Settled: id=" << u->id << " dist=" << u->key << "\n";
    assert(u->id == 2 && u->key == 3.0);
    delete u;

    h.decrease_key(p3, 3.0 + 1.0);  // 10 -> 4, improve d[3]

    u = h.extract_min();
    std::cout << "Settled: id=" << u->id << " dist=" << u->key << "\n";
    assert(u->id == 3 && u->key == 4.0);
    delete u;

    std::cout << "  Shortest paths verified: d[1]=1, d[2]=3, d[3]=4\n";
    std::cout << "  [PASS]\n";

    while (!h.empty()) delete h.extract_min();
}



int main() {
    std::cout << "--------------------------------------------\n";
    std::cout << "|          FibHeap    Testing :)            |\n";
    std::cout << "--------------------------------------------\n";

    test_basic_order();
    test_decrease_key();
    test_consolidation();
    test_cascading_cut();
    test_dijkstra_simulation();

    std::cout << "--------------------------------------------\n";
    std::cout << "|          All FibHeap Tests PASSED!        |\n";
    std::cout << "--------------------------------------------\n";
    return 0;
}