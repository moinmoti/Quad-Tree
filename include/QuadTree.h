#pragma once

#include "Node.h"

struct QuadTree {

    Node *root;

    QuadTree(uint, Rect, Split);
    ~QuadTree();

    void bulkload(string, long);
    Info deleteQuery(Record);
    Info insertQuery(Record);
    Info kNNQuery(Data, uint);
    Info rangeQuery(Rect);
    uint size(map<string, double> &) const;
    void snapshot() const;
};
