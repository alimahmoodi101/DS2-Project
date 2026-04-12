#pragma once
#include <vector>
#include <cmath>
#include <limits>

// ---------------------------------------------------------------------------
//  FibHeap  –  Fibonacci Heap (min-heap variant)
//
//  Complexity:
//    insert        O(1) amortised
//    decrease_key  O(1) amortised
//    extract_min   O(log n) amortised
//
//  Each node stores:
//    key      – current shortest distance
//    id       – graph node ID
//    degree   – number of children
//    mark     – cascading-cut flag
//    child, parent, left, right  – pointer structure
//
//  The caller keeps a pointer to each inserted node so that decrease_key
//  can operate in O(1) without a search.
// ---------------------------------------------------------------------------

struct FibNode {
    double   key;
    int      id;
    int      degree = 0;
    bool     mark   = false;

    FibNode* parent = nullptr;
    FibNode* child  = nullptr;
    FibNode* left   = nullptr;   // circular doubly-linked list
    FibNode* right  = nullptr;
};

class FibHeap {
public:
    FibHeap();
    ~FibHeap();

    // Insert a new node; returns a raw pointer the caller must hold for
    // decrease_key.  Ownership stays with the heap.
    FibNode* insert(int id, double key);

    // Lower node's key to `key`; node must currently be in this heap.
    void decrease_key(FibNode* node, double key);

    // Remove and return the minimum node.  Caller must delete the pointer.
    FibNode* extract_min();

    bool empty()    const;
    int  size()     const;

    // Debug: print the root list (one level deep)
    void print_roots() const;

private:
    FibNode* min_  = nullptr;
    int      n_    = 0;

    // Splice a single node into the root list (updates min_ if needed)
    void add_to_root_list(FibNode* x);

    // Remove a node from whatever circular list it currently sits in
    void remove_from_list(FibNode* x);

    // Link y as a child of x (both already in root list, y.key >= x.key)
    void link(FibNode* y, FibNode* x);

    // Consolidate: merge trees of equal degree after extract_min
    void consolidate();

    // Cut x from its parent's child list and add to root list
    void cut(FibNode* x, FibNode* parent);

    // Cascading cut up the ancestor chain
    void cascading_cut(FibNode* y);

    // Recursively delete an entire subtree (used in destructor)
    void delete_tree(FibNode* x);
};