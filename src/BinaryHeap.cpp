#include "BinaryHeap.h"
#include <iostream>
#include <stdexcept>
#include <cmath>

static constexpr double INF = std::numeric_limits<double>::infinity();

// ── constructor ──────────────────────────────────────────────────────────────
BinaryHeap::BinaryHeap(int n)
    : heap_(1), best_(n, INF), size_(0), live_(0)
{
    heap_.reserve(n + 1);
}

// ── insert ───────────────────────────────────────────────────────────────────
void BinaryHeap::insert(int id, double key) {
    if (best_[id] < 0)       return;   // already settled
    if (key >= best_[id])    return;   // not an improvement

    bool first_time = (best_[id] == INF);
    best_[id] = key;

    heap_.push_back({key, id});
    ++size_;
    sift_up(size_);

    if (first_time) ++live_;           // new unsettled node discovered
}

// ── decrease_key ─────────────────────────────────────────────────────────────
void BinaryHeap::decrease_key(int id, double key) {
    insert(id, key);
}

// ── extract_min ──────────────────────────────────────────────────────────────
BHNode BinaryHeap::extract_min() {
    if (empty()) throw std::runtime_error("extract_min on empty heap");

    while (size_ > 0) {
        // Pop root
        BHNode top = heap_[1];
        heap_[1] = heap_[size_];
        heap_.pop_back();
        --size_;
        if (size_ > 0) sift_down(1);

        // Skip if settled
        if (best_[top.id] < 0) continue;

        // Skip if superseded (a better push happened after this one)
        if (std::abs(top.key - best_[top.id]) > 1e-9) continue;

        // Live entry — settle it
        best_[top.id] = -1.0;
        --live_;
        return top;
    }

    // Should never reach here if empty() was checked before calling
    throw std::runtime_error("extract_min: no live entries remaining");
}

// ── empty ────────────────────────────────────────────────────────────────────
bool BinaryHeap::empty() const { return live_ == 0; }

double BinaryHeap::dist(int id) const { return best_[id]; }

// ── print ─────────────────────────────────────────────────────────────────────
void BinaryHeap::print() const {
    std::cout << "[BinaryHeap] raw_size=" << size_
              << "  live=" << live_ << "  (1-indexed)\n";
    for (int i = 1; i <= size_; ++i)
        std::cout << "  [" << i << "] id=" << heap_[i].id
                  << "  key=" << heap_[i].key << "\n";
}

// ── sift_up / sift_down ──────────────────────────────────────────────────────
void BinaryHeap::sift_up(int pos) {
    while (pos > 1) {
        int parent = pos / 2;
        if (heap_[parent] > heap_[pos]) {
            std::swap(heap_[parent], heap_[pos]);
            pos = parent;
        } else break;
    }
}

void BinaryHeap::sift_down(int pos) {
    while (true) {
        int smallest = pos;
        int left  = 2 * pos;
        int right = 2 * pos + 1;
        if (left  <= size_ && heap_[left].key  < heap_[smallest].key) smallest = left;
        if (right <= size_ && heap_[right].key < heap_[smallest].key) smallest = right;
        if (smallest == pos) break;
        std::swap(heap_[pos], heap_[smallest]);
        pos = smallest;
    }
}