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
    pRoot->points.reserve(limit);
    if (file.is_open()) {
        // getline(file, line);
        while (getline(file, line)) {
            int id;
            float lat, lon;
            istringstream buf(line);
            buf >> id >> lon >> lat;
            array pt{lon, lat};
            pRoot->points.emplace_back(Record{.id = id, .data = pt});
            if (++i >= limit)
                break;
        }
        file.close();
    } else
        cerr << "Data file " << filename << " not found!";

    cout << "Initiate fission" << endl;
    if (pRoot->points.size() > Page::capacity)
        root = pRoot->fission();
}

Info QuadTree::deleteQuery(Record p) {
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

Info QuadTree::insertQuery(Record p) {
    Info info;
    root = root->insert(p, info.cost);
    return info;
}

Info QuadTree::kNNQuery(Data p, uint k) {
    Info info;
    array query{p[0], p[1], p[0], p[1]};

    min_heap<Node::knnNode> unseenNodes;
    vector<Node::knnPoint> tempPts(k);
    max_heap<Node::knnPoint> knnPts(all(tempPts));
    Node *node = root;
    unseenNodes.emplace(Node::knnNode{node, node->minSqrDist(query)});
    double dist, minDist;
    while (!unseenNodes.empty()) {
        node = unseenNodes.top().sn;
        dist = unseenNodes.top().dist;
        unseenNodes.pop();
        minDist = knnPts.top().dist;
        if (dist < minDist)
            info.cost += node->knnSearch(query, unseenNodes, knnPts);
        else
            break;
    }

    /* double sqrDist;
    if (k == 32) {
        while (!knnPts.empty()) {
            Record pt = knnPts.top().pt;
            sqrDist = knnPts.top().dist;
            knnPts.pop();
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

uint QuadTree::size(map<string, double> &stats) const {
    uint totalSize = 2 * sizeof(uint);
    uint pageSize = 4 * sizeof(float) + sizeof(uint) + sizeof(void *);
    uint directorySize = 4 * sizeof(float) + sizeof(uint) + sizeof(void *);
    stack<Directory *> toVisit({static_cast<Directory *>(root)});
    Directory *dir;
    while (!toVisit.empty()) {
        dir = toVisit.top();
        toVisit.pop();
        stats["directories"]++;
        for (auto cn : dir->quartet) {
            Directory *dcn = dynamic_cast<Directory *>(cn);
            if (dcn) {
                toVisit.push(dcn);
            } else
                stats["pages"]++;
        }
    }
    totalSize += pageSize * stats["pages"] + directorySize * stats["directories"];
    return totalSize;
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
    ofstream log(splitStr + "-QuadTree.csv");
    stack<Directory *> toVisit({static_cast<Directory *>(root)});
    Directory *dir;
    while (!toVisit.empty()) {
        dir = toVisit.top();
        toVisit.pop();
        // log << dir->height << "," << dir->quartet.size();
        for (auto p : dir->rect)
            log << "," << p;
        log << endl;
        for (auto cn : dir->quartet) {
            Directory *dcn = dynamic_cast<Directory *>(cn);
            if (dcn) {
                toVisit.push(dcn);
            } else {
                log << 0 << "," << static_cast<Page *>(cn)->points.size();
                for (auto p : cn->rect)
                    log << "," << p;
                log << endl;
            }
        }
    }
    log.close();
}
