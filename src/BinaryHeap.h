#pragma once
#include <vector>
#include <limits>

// ---------------------------------------------------------------------------
//  BinaryHeap  –  min-heap with lazy deletion
//
//  Each entry is a (key, id) pair.  "Lazy deletion" means decrease-key pushes
//  a fresh entry with the lower key rather than updating in place.
//  extract_min skips stale entries (those superseded by a later push or
//  already settled) until it finds a live one.
//
//  empty() returns true when no LIVE (unsettled) entries remain, even if
//  stale entries are still sitting in the raw heap array.
//
//  API used by Dijkstra:
//    insert(id, key)       – push entry; ignores if settled or not improvement
//    decrease_key(id, key) – alias for insert (lazy)
//    extract_min()         – pop and return the live minimum
//    empty()               – true when all live entries are exhausted
// ---------------------------------------------------------------------------

struct BHNode {
    double key;
    int    id;
    bool operator>(const BHNode& o) const { return key > o.key; }
};

class BinaryHeap {
public:
    explicit BinaryHeap(int n);   // n = number of distinct node IDs

    void   insert(int id, double key);
    void   decrease_key(int id, double key);
    BHNode extract_min();
    bool   empty() const;

    double dist(int id) const;
    void   print() const;

private:
    std::vector<BHNode>  heap_;   // 1-indexed; heap_[0] unused
    std::vector<double>  best_;   // best known key per id (INF=unseen, -1=settled)
    int                  size_;   // raw array size (includes stale entries)
    int                  live_;   // count of unsettled node IDs

    void sift_up(int pos);
    void sift_down(int pos);
};