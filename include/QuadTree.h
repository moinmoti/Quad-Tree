#pragma once

#include "Node.h"

struct QuadTree {

    Node *root;

    QuadTree(uint, Rect, Split);
    ~QuadTree();

    void bulkload(string, long);
    Info deleteQuery(Entry);
    Info insertQuery(Entry);
    Info kNNQuery(Point, uint);
    Info rangeQuery(Rect);
    uint size(array<uint, 2> &) const;
    void snapshot() const;
};
