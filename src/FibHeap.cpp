#include "FibHeap.h"
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cmath>

// ── constructor / destructor ─────────────────────────────────────────────────
FibHeap::FibHeap() : min_(nullptr), n_(0) {}

FibHeap::~FibHeap() {
    if (!min_) return;

    // Collect all roots then recursively delete each tree
    std::vector<FibNode*> roots;
    FibNode* cur = min_;
    do {
        roots.push_back(cur);
        cur = cur->right;
    } while (cur != min_);

    for (FibNode* r : roots) delete_tree(r);
}

// ── insert ───────────────────────────────────────────────────────────────────
FibNode* FibHeap::insert(int id, double key) {
    FibNode* x  = new FibNode();
    x->id       = id;
    x->key      = key;
    x->left     = x;       // self-referential circular list (single node)
    x->right    = x;

    add_to_root_list(x);
    ++n_;
    return x;
}

// ── decrease_key ─────────────────────────────────────────────────────────────
void FibHeap::decrease_key(FibNode* x, double key) {
    if (key > x->key)
        throw std::invalid_argument("decrease_key: new key is larger than current key");

    x->key = key;
    FibNode* parent = x->parent;

    if (parent && x->key < parent->key) {
        cut(x, parent);
        cascading_cut(parent);
    }

    if (x->key < min_->key) min_ = x;
}

// ── extract_min ──────────────────────────────────────────────────────────────
FibNode* FibHeap::extract_min() {
    if (!min_) throw std::runtime_error("extract_min on empty FibHeap");

    FibNode* z = min_;

    // Move all of z's children into the root list
    if (z->child) {
        std::vector<FibNode*> children;
        FibNode* c = z->child;
        do {
            children.push_back(c);
            c = c->right;
        } while (c != z->child);

        for (FibNode* ch : children) {
            remove_from_list(ch);
            add_to_root_list(ch);
            ch->parent = nullptr;
        }
    }

    // Remove z from root list
    remove_from_list(z);

    if (z == z->right) {
        // z was the only root
        min_ = nullptr;
    } else {
        min_ = z->right;   // temporary; consolidate will find true min
        consolidate();
    }

    --n_;
    z->left = z->right = z;    // isolate node before returning
    z->parent = z->child = nullptr;
    return z;
}

bool FibHeap::empty() const { return n_ == 0; }
int  FibHeap::size()  const { return n_; }

// ── debug print ──────────────────────────────────────────────────────────────
void FibHeap::print_roots() const {
    if (!min_) { std::cout << "[FibHeap] (empty)\n"; return; }

    std::cout << "[FibHeap] n=" << n_ << "  min=id" << min_->id
              << "(key=" << min_->key << ")  root list:\n";

    FibNode* cur = min_;
    do {
        std::cout << "  root id=" << cur->id
                  << "  key="     << cur->key
                  << "  degree="  << cur->degree
                  << "  mark="    << cur->mark << "\n";
        cur = cur->right;
    } while (cur != min_);
}

// ── private helpers ──────────────────────────────────────────────────────────

void FibHeap::add_to_root_list(FibNode* x) {
    if (!min_) {
        x->left = x->right = x;
        min_ = x;
    } else {
        // Insert x to the left of min_ in the circular list
        x->right        = min_;
        x->left         = min_->left;
        min_->left->right = x;
        min_->left      = x;

        if (x->key < min_->key) min_ = x;
    }
    x->parent = nullptr;
}

void FibHeap::remove_from_list(FibNode* x) {
    x->left->right = x->right;
    x->right->left = x->left;
    // Do NOT clear x->left/right here — callers may still need to traverse
}

void FibHeap::link(FibNode* y, FibNode* x) {
    // Make y a child of x
    remove_from_list(y);
    y->parent = x;

    if (!x->child) {
        x->child  = y;
        y->left   = y;
        y->right  = y;
    } else {
        // Insert y into x's child list
        y->right            = x->child;
        y->left             = x->child->left;
        x->child->left->right = y;
        x->child->left      = y;
    }

    ++x->degree;
    y->mark = false;
}

void FibHeap::consolidate() {
    // Max degree bound: floor(log_phi(n)) + 2
    int max_degree = static_cast<int>(std::log(n_) / std::log(1.618)) + 2;
    std::vector<FibNode*> A(max_degree + 1, nullptr);

    // Collect all roots first to avoid iterator invalidation
    std::vector<FibNode*> roots;
    FibNode* cur = min_;
    do {
        roots.push_back(cur);
        cur = cur->right;
    } while (cur != min_);

    for (FibNode* w : roots) {
        FibNode* x = w;
        int d = x->degree;

        while (d <= max_degree && A[d]) {
            FibNode* y = A[d];
            if (x->key > y->key) std::swap(x, y);
            link(y, x);     // y becomes child of x
            A[d] = nullptr;
            ++d;
        }

        if (d > max_degree) {
            // Resize A if necessary (shouldn't normally happen, but be safe)
            A.resize(d + 1, nullptr);
        }
        A[d] = x;
    }

    // Rebuild root list from A and find new min
    min_ = nullptr;
    for (FibNode* node : A) {
        if (!node) continue;
        node->parent = nullptr;
        if (!min_) {
            node->left = node->right = node;
            min_ = node;
        } else {
            // Insert into root list
            node->right        = min_;
            node->left         = min_->left;
            min_->left->right  = node;
            min_->left         = node;
            if (node->key < min_->key) min_ = node;
        }
    }
}

void FibHeap::cut(FibNode* x, FibNode* p) {
    // Remove x from p's child list
    if (x->right == x) {
        p->child = nullptr;
    } else {
        if (p->child == x) p->child = x->right;
        remove_from_list(x);
    }
    --p->degree;

    // Add x to root list
    add_to_root_list(x);
    x->mark = false;
}

void FibHeap::cascading_cut(FibNode* y) {
    FibNode* p = y->parent;
    if (!p) return;         // y is already a root

    if (!y->mark) {
        y->mark = true;     // first time a child has been cut — just mark
    } else {
        cut(y, p);          // second time — cut and continue upward
        cascading_cut(p);
    }
}

void FibHeap::delete_tree(FibNode* x) {
    if (!x) return;
    if (x->child) {
        std::vector<FibNode*> children;
        FibNode* c = x->child;
        do {
            children.push_back(c);
            c = c->right;
        } while (c != x->child);
        for (FibNode* ch : children) delete_tree(ch);
    }
    delete x;
}