#include "BinaryHeap.h"
#include <iostream>
#include <stdexcept>
#include <cmath>

static constexpr double INF = std::numeric_limits<double>::infinity();

// ── constructor ─────────────────────────────────────────────────────────────
BinaryHeap::BinaryHeap(int n)
    : heap_(1), dist_(n, INF), size_(0)
{
    heap_.reserve(n + 1);   // heap_[0] is a sentinel / unused slot
}

// ── public ───────────────────────────────────────────────────────────────────
void BinaryHeap::insert(int id, double key) {
    dist_[id] = key;
    heap_.push_back({key, id});
    ++size_;
    sift_up(size_);
}

void BinaryHeap::decrease_key(int id, double key) {
    if (dist_[id] < 0) return;       // already settled, ignore
    if (key >= dist_[id]) return;    // not an improvement
    dist_[id] = key;
    heap_.push_back({key, id});
    ++size_;
    sift_up(size_);
}

BHNode BinaryHeap::extract_min() {
    if (empty()) throw std::runtime_error("extract_min on empty heap");

    // Lazy deletion: pop entries until we find one whose stored key matches
    // the canonical dist_ value (i.e. it has not been superseded by a
    // decrease_key call that pushed a fresher entry).
    while (!empty()) {
        BHNode top = heap_[1];
        heap_[1] = heap_[size_];
        heap_.pop_back();
        --size_;
        if (size_ > 0) sift_down(1);

        // Stale check: if dist_[id] has been beaten by a later decrease_key,
        // the canonical value will differ from this entry's key.
        // We use a small epsilon to handle floating-point equality.
        if (std::abs(top.key - dist_[top.id]) < 1e-9) {
            // Live entry — mark as settled with a negative sentinel
            dist_[top.id] = -1.0;
            return top;
        }
        // else stale: skip and continue
    }
    throw std::runtime_error("extract_min: no live entries remaining");
}

bool BinaryHeap::empty() const { return size_ == 0; }

double BinaryHeap::dist(int id) const { return dist_[id]; }

void BinaryHeap::print() const {
    std::cout << "[BinaryHeap] size=" << size_ << "  (1-indexed raw array)\n";
    for (int i = 1; i <= size_; ++i) {
        std::cout << "  [" << i << "] id=" << heap_[i].id
                  << "  key=" << heap_[i].key << "\n";
    }
}

// ── private helpers ──────────────────────────────────────────────────────────
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

        if (left  <= size_ && heap_[left].key < heap_[smallest].key)
            smallest = left;
        if (right <= size_ && heap_[right].key < heap_[smallest].key)
            smallest = right;

        if (smallest == pos) break;
        std::swap(heap_[pos], heap_[smallest]);
        pos = smallest;
    }
}