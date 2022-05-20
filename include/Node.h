#pragma once

#include "common.h"

struct Node {

    struct knnEntry {
        Entry pt;
        double dist = numeric_limits<double>::max();
        bool operator<(const knnEntry &second) const { return dist < second.dist; }
    };

    struct knnNode {
        Node *sn;
        double dist = numeric_limits<double>::max();
        bool operator>(const knnNode &second) const { return dist > second.dist; }
    };

    Rect rect; // xlow, ylow, xhigh, yhigh

    // Rect methods
    bool containsPt(Point p) const;
    Point getCenter() const;
    bool inside(Rect) const;
    double minSqrDist(Rect) const;
    bool overlap(Rect) const;

    virtual Node* insert(Entry, uint &) = 0;
    virtual uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnEntry> &) const = 0;
    virtual uint range(uint &, Rect query) const = 0;
    virtual uint size(array<uint, 2> &) const = 0;
    virtual uint snapshot(ofstream &) const = 0;

    virtual ~Node() = 0;
};

struct Directory: Node {
    array<Node*, 4> quartet; // One for each quarter

    Directory();
    explicit Directory(Node *, bool = true);

    Node* insert(Entry, uint &);
    uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnEntry> &) const;
    uint range(uint &, Rect query) const;
    uint size(array<uint, 2> &) const;
    uint snapshot(ofstream &) const;

    ~Directory();
};

struct Page: Node {
    static uint capacity;
    static Split split;
    vector<Entry> entries;

    Page();
    explicit Page(Node *, bool = true);

    Node* fission();
    Point getSplit();
    Node* insert(Entry, uint &);
    uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnEntry> &) const;
    array<Node*, 4> partition(uint &);
    uint range(uint &, Rect) const;
    uint size(array<uint, 2> &) const;
    uint snapshot(ofstream &) const;

    ~Page();
};
