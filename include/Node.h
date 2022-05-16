#pragma once

#include "common.h"

struct Node {

    struct knnPoint {
        Record pt;
        double dist = numeric_limits<double>::max();
        bool operator<(const knnPoint &second) const { return dist < second.dist; }
    };

    struct knnNode {
        Node *sn;
        double dist = numeric_limits<double>::max();
        bool operator>(const knnNode &second) const { return dist > second.dist; }
    };

    Rect rect; // xlow, ylow, xhigh, yhigh

    // Rect methods
    bool containsPt(Data p) const;
    Data getCenter() const;
    bool inside(Rect) const;
    double minSqrDist(Rect) const;
    bool overlap(Rect) const;

    virtual Node* insert(Record, uint &) = 0;
    virtual uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnPoint> &) const = 0;
    virtual uint range(uint &, Rect query) const = 0;
    virtual uint size() const = 0;

    virtual ~Node() = 0;
};

struct Directory: Node {
    array<Node*, 4> quartet; // One for each quarter

    Directory();
    explicit Directory(Node *, bool = true);

    Node* insert(Record, uint &);
    uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnPoint> &) const;
    uint range(uint &, Rect query) const;
    uint size() const;

    ~Directory();
};

struct Page: Node {
    static uint capacity;
    static Split split;
    vector<Record> points;

    Page();
    explicit Page(Node *, bool = true);

    Node* fission();
    Data getSplit() const;
    Node* insert(Record, uint &);
    uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnPoint> &) const;
    array<Node*, 4> partition(uint &);
    uint range(uint &, Rect) const;
    uint size() const;

    ~Page();
};
