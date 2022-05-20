#include "QuadTree.h"

void printRect(string str, Rect r) {
    cerr << str << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

QuadTree::QuadTree(uint _capacity, Rect _boundary, Split _split) {
    Page::capacity = _capacity;
    Page::split = _split;

    root = new Page();
    root->rect = _boundary;
}

QuadTree::~QuadTree() {}

void QuadTree::bulkload(string filename, long limit) {
    string line;
    ifstream file(filename);

    int i = 0;
    Page *pRoot = static_cast<Page *>(root);
    pRoot->entries.reserve(limit);
    if (file.is_open()) {
        // getline(file, line);
        while (getline(file, line)) {
            int id;
            float lat, lon;
            istringstream buf(line);
            buf >> id >> lon >> lat;
            array pt{lon, lat};
            pRoot->entries.emplace_back(Entry{.id = id, .pt = pt});
            if (++i >= limit)
                break;
        }
        file.close();
    } else
        cerr << "Point file " << filename << " not found!";

    cout << "Initiate fission" << endl;
    if (pRoot->entries.size() > Page::capacity)
        root = pRoot->fission();
}

Info QuadTree::deleteQuery(Entry p) {
    Info info;
    /* Node *node = root;
    while (node->height) {
        auto cn = static_cast<Directory *>(node)->quartet.begin();
        while (!(*cn)->containsPt(p.data))
            cn++;
        node = *cn;
    } */
    /* auto pt = find(all(node->points.value()), p);
    if (pt != node->points->end())
        node->points->erase(pt); */
    return info;
}

Info QuadTree::insertQuery(Entry p) {
    Info info;
    root = root->insert(p, info.cost);
    return info;
}

Info QuadTree::kNNQuery(Point p, uint k) {
    Info info;
    array query{p[0], p[1], p[0], p[1]};

    min_heap<Node::knnNode> unseenNodes;
    vector<Node::knnEntry> tempEnts(k);
    max_heap<Node::knnEntry> knnEnts(all(tempEnts));
    Node *node = root;
    unseenNodes.emplace(Node::knnNode{node, node->minSqrDist(query)});
    double dist, minDist;
    while (!unseenNodes.empty()) {
        node = unseenNodes.top().sn;
        dist = unseenNodes.top().dist;
        unseenNodes.pop();
        minDist = knnEnts.top().dist;
        if (dist < minDist)
            info.cost += node->knnSearch(query, unseenNodes, knnEnts);
        else
            break;
    }

    /* double sqrDist;
    if (k == 32) {
        while (!knnEnts.empty()) {
            Entry pt = knnEnts.top().pt;
            sqrDist = knnEnts.top().dist;
            knnEnts.pop();
            trace(pt.id, sqrDist);
        }
        cerr << endl;
    } */
    return info;
}

Info QuadTree::rangeQuery(Rect query) {
    Info info;
    info.cost = root->range(info.output, query);
    /* int pointCount = info.output;
    trace(pointCount); */
    return info;
}

uint QuadTree::size(array<uint, 2> &stats) const {
    stats = {0, 0};
    return root->size(stats);
}

void QuadTree::snapshot() const {
    string splitStr;
    Split usedType = Page::split;
    if (usedType == X)
        splitStr = "X";
    else if (usedType == Y)
        splitStr = "Y";
    else if (usedType == Orientation)
        splitStr = "Orientation";
    else if (usedType == Center)
        splitStr = "Center";
    else if (usedType == Cross)
        splitStr = "Cross";
    else
        splitStr = "Invalid";
    ofstream ofs(splitStr + "-QuadTree.csv");
    root->snapshot(ofs);
    ofs.close();
}
