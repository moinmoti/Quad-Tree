#include "Node.h"

bool overlaps(Rect r, Data p) {
    for (uint i = 0; i < D; i++) {
        if (r[i] > p[i] || p[i] > r[i + D])
            return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Node Methods
/////////////////////////////////////////////////////////////////////////////////////////

void printNode(string str, Rect r) {
    cerr << str << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

bool Node::overlap(Rect r) const {
    for (uint i = 0; i < D; i++)
        if (rect[i] > r[i + D] || r[i] > rect[i + D])
            return false;
    return true;
}

bool Node::containsPt(Data p) const {
    bool result = true;
    for (uint i = 0; i < D; i++)
        result = result & (rect[i] <= p[i]) & (rect[i + D] >= p[i]);
    return result;
}

bool Node::inside(Rect r) const {
    bool result = true;
    for (uint i = 0; i < D; i++)
        result = result & (rect[i] >= r[i]) & (rect[i + D] <= r[i + D]);
    return result;
}

Data Node::getCenter() const { return Data{(rect[0] + rect[2]) / 2, (rect[1] + rect[3]) / 2}; }

double Node::minSqrDist(Rect r) const {
    bool left = r[2] < rect[0];
    bool right = rect[2] < r[0];
    bool bottom = r[3] < rect[1];
    bool top = rect[3] < r[1];
    if (top) {
        if (left)
            return dist(rect[0], rect[3], r[2], r[1]);
        if (right)
            return dist(rect[2], rect[3], r[0], r[1]);
        return (r[1] - rect[3]) * (r[1] - rect[3]);
    }
    if (bottom) {
        if (left)
            return dist(rect[0], rect[1], r[2], r[3]);
        if (right)
            return dist(rect[2], rect[1], r[0], r[3]);
        return (rect[1] - r[3]) * (rect[1] - r[3]);
    }
    if (left)
        return (rect[0] - r[2]) * (rect[0] - r[2]);
    if (right)
        return (r[0] - rect[2]) * (r[0] - rect[2]);
    return 0;
}

Node::~Node() {}

/////////////////////////////////////////////////////////////////////////////////////////
// Directory Methods
/////////////////////////////////////////////////////////////////////////////////////////

Directory::Directory() {}

Directory::Directory(Node *pg, bool canDel) {
    rect = pg->rect;
    if (canDel)
        delete pg;
}

Node *Directory::insert(Record p, uint &writes) {
    for (auto &qn : quartet) {
        if (qn->containsPt(p.data)) {
            qn = qn->insert(p, writes);
            break;
        }
    }
    return this;
}

uint Directory::knnSearch(Rect query, min_heap<knnNode> &unseenNodes,
                          max_heap<knnPoint> &knnPts) const {
    double minDist = knnPts.top().dist;
    for (auto cn : quartet) {
        double dist = cn->minSqrDist(query);
        if (dist < minDist) {
            knnNode kn;
            kn.sn = cn;
            kn.dist = dist;
            unseenNodes.push(kn);
        }
    }
    return 0;
}

uint Directory::range(uint &pointCount, Rect query) const {
    uint reads = 0;
    for (auto qn : quartet) {
        if (qn->overlap(query))
            reads += qn->range(pointCount, query);
    }
    return reads;
}

uint Directory::size() const {
    uint rectSize = sizeof(float) * 4;
    uint typeSize = sizeof(vector<Node *>) + quartet.size() * sizeof(void *);
    uint totalSize = typeSize + rectSize;
    return totalSize;
}

Directory::~Directory() {
    for (auto qn : quartet) {
        delete qn;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// Page Methods
/////////////////////////////////////////////////////////////////////////////////////////

uint Page::capacity;
Split Page::split;

Page::Page() {}

Page::Page(Node *dir, bool canDel) {
    rect = dir->rect;
    if (canDel)
        delete dir;
}

Node *Page::fission() {
    uint writes = 0;
    Node *node = new Directory(this, false);
    Directory *dir = static_cast<Directory *>(node);
    dir->quartet = partition(writes);
    for (auto &cn : dir->quartet) {
        Page *pg = static_cast<Page *>(cn);
        if (pg->points.size() > capacity)
            cn = pg->fission();
    }
    delete this;
    return node;
}

Data Page::getSplit() const {
    bool axis;
    if (split == X)
        axis = false;
    else if (split == Y)
        axis = true;
    else if (split == Orientation)
        axis = (rect[2] - rect[0]) - (rect[3] - rect[1]);
    if (split == Spread) {
        Data low({180, 90}), high({-180, -90});
        for (auto p : points) {
            if (p.data[0] < low[0])
                low[0] = p.data[0];
            if (p.data[1] < low[1])
                low[1] = p.data[1];
            if (p.data[0] > high[0])
                high[0] = p.data[0];
            if (p.data[1] > high[1])
                high[1] = p.data[1];
        }
        axis = (high[0] - low[0]) < (high[1] - low[1]);
    } else {
        if (split == Center) {
            return getCenter();
        } else if (split == Cross) {
            Data crossPt;
            sort(all(points),
                 [](const Record &l, const Record &r) { return l.data[0] < r.data[0]; });
            crossPt[0] = points[points.size() / 2].data[0];
            sort(all(points),
                 [](const Record &l, const Record &r) { return l.data[1] < r.data[1]; });
            crossPt[1] = points[points.size() / 2].data[1];
            return crossPt;
        } else {
            cerr << "Error: Invalid TYPE!!!" << endl;
            exit(EXIT_FAILURE);
        }
    }
    sort(all(points),
         [axis](const Record &l, const Record &r) { return l.data[axis] < r.data[axis]; });
    return points[points.size() / 2].data;
}

Node *Page::insert(Record p, uint &writes) {
    points.emplace_back(p);
    if (points.size() > capacity) {
        Node *node = new Directory(this, false);
        Directory *dir = static_cast<Directory *>(node);
        dir->quartet = partition(writes);
        delete this;
        return node;
    }
    writes += 2;
    return this;
}

uint Page::knnSearch(Rect query, min_heap<knnNode> &unseenNodes, max_heap<knnPoint> &knnPts) const {
    auto calcSqrDist = [](Rect x, Data y) { return pow((x[0] - y[0]), 2) + pow((x[1] - y[1]), 2); };
    for (auto p : points) {
        double minDist = knnPts.top().dist;
        double dist = calcSqrDist(query, p.data);
        if (dist < minDist) {
            knnPoint kPt;
            kPt.pt = p;
            kPt.dist = dist;
            knnPts.pop();
            knnPts.push(kPt);
        }
    }
    return 1;
}

array<Node *, 4> Page::partition(uint &writes) {
    array<Node *, 4> pages;
    Data splitPt = getSplit();
    for (uint i = 0; i < pages.size(); i++) {
        pages[i] = new Page();
        pages[i]->rect = rect;
        bitset<D> b(i);
        for (uint k = 0; k < D; k++)
            pages[i]->rect[k + (!b[k]) * (1 << k)] = splitPt[k];
    }

    // Splitting points
    for (auto p : points) {
        uint minCount = INT_MAX;
        Page *container;
        for (auto nd : pages) {
            Page *pg = static_cast<Page *>(nd);
            if (pg->containsPt(p.data)) {
                if (minCount > pg->points.size()) {
                    minCount = pg->points.size();
                    container = pg;
                }
            }
        }
        container->points.emplace_back(p);
    }

    points.clear();
    writes += 3;
    return pages;
}

uint Page::range(uint &pointCount, Rect query) const {
    if (inside(query))
        pointCount += points.size();
    else {
        for (auto p : points)
            if (overlaps(query, p.data))
                pointCount++;
    }
    return 1;
}

uint Page::size() const {
    uint rectSize = sizeof(float) * 4;
    uint typeSize = sizeof(vector<Data>);
    uint totalSize = typeSize + rectSize;
    return totalSize;
}

Page::~Page() { points.clear(); }
