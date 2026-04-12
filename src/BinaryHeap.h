#pragma once
#include <vector>
#include <limits>

// ---------------------------------------------------------------------------
//  BinaryHeap  –  min-heap with lazy deletion
//
//  Each entry is a (key, id) pair.  "Lazy deletion" means decrease-key does
//  NOT update an existing entry in place; instead it pushes a fresh entry
//  with the lower key and marks the old one as stale.  extract-min skips any
//  popped entry whose stored key no longer matches the canonical distance for
//  that id (tracked in dist[]).
//
//  API used by Dijkstra:
//    insert(id, key)         – add a brand-new node
//    decrease_key(id, key)   – push a new entry (lazy)
//    extract_min()           – returns {id, key}, skipping stale entries
//    empty()
// ---------------------------------------------------------------------------

struct BHNode {
    double key;
    int    id;

    // min-heap comparator
    bool operator>(const BHNode& o) const { return key > o.key; }
};

class BinaryHeap {
public:
    explicit BinaryHeap(int n);          // n = number of distinct node IDs

    void   insert(int id, double key);
    void   decrease_key(int id, double key);
    BHNode extract_min();
    bool   empty() const;

    // Current best-known key for id (-1 → not yet inserted)
    double dist(int id) const;

    // Pretty-print the raw heap array (for testing)
    void print() const;

private:
    std::vector<BHNode>   heap_;    // 1-indexed; heap_[0] unused
    std::vector<double>   dist_;   // canonical distance per id
    int                   size_;

    void sift_up(int pos);
    void sift_down(int pos);
};