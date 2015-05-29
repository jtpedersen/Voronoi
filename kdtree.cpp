#include "kdtree.h"

#include <stdexcept>

void KDTree::insert(const KDNode& n) {
    nodes.emplace_back(n);
    if(nodes.size() == 1) {
	// a new root will split on X
	nodes[0].axis = 0;
    } else {
	insertLastInTree(nodes[0]);
    }
}

void KDTree::insertLeft(KDNode& r) {
    if (r.left < 0) {
	r.left = nodes.size() -1;
	nodes.back().axis = (r.axis == 0) ? 1 : 0;
    } else {
	insertLastInTree(nodes[r.left]);
    }
}

void KDTree::insertRight(KDNode& r) {
    if (r.right < 0) {
	r.right = nodes.size() -1;
	nodes.back().axis = (r.axis == 0) ? 1 : 0;
    } else {
	insertLastInTree(nodes[r.right]);
    }
}

bool KDTree::leftOf(const KDNode& r, const KDNode& n) const {
    return leftOf(r, n.p);
}

bool KDTree::leftOf(const KDNode& r, const vec2& p) const {
    if (r.axis == 0) {
	return (p.x < r.p.x);
    } else if (r.axis == 1) {
	return (p.y < r.p.y);
    }
    
    assert(0);
    return false;
}

void KDTree::insertLastInTree(KDNode& r) {
    const auto& n = nodes.back();
    if (leftOf(r, n)) {
	insertLeft(r);
    } else { 
	insertRight(r);
    }
}

const KDNode& KDTree::findNearest(const vec2& p) const {
    if (nodes.empty())
	throw std::runtime_error("no nodes in tree");

    float r2 = 10e99;
    int best = -1;
    nnsearch(p, 0, r2, best);
    //cout << "nnsearched for: " << to_string(p) << " (r2=" << r2 << ", best=" << best << ")" << endl;

    return nodes[best];
}

void KDTree::nnsearch(const vec2& p, int root, float &r2, int& best) const {
    if (root == -1) return;
//    cout << "nnsearch for: " << to_string(p) << "@" << root << " (r2=" << r2 << ", best=" << best << ")" << endl;
    float d2 = distance2(p, nodes[root].p);
    if (d2 < r2) {
	r2 = d2;
	best = root;
    }
    auto cur = nodes[root];
    const bool goLeft = leftOf(cur, p);
    int thisSide = goLeft ? cur.left : cur.right;
    
    nnsearch(p, thisSide, r2, best);

    float distanceToSplitPlane = (cur.axis == 0) ? p.x - cur.p.x : p.y - cur.p.y;
    if (distanceToSplitPlane*distanceToSplitPlane < r2 ) {
	int otherSide = goLeft ? cur.right : cur.left;
	nnsearch(p, otherSide, r2, best);
    }
}

const KDNode& KDTree::operator[] (size_t idx) const {
    if (nodes.empty())
	throw std::runtime_error("no nodes in tree");
    return nodes[idx];
}


void KDTree::clear() {
    nodes.clear();
}

void KDTree::dumpNodes(ostream& os) {
    for (auto n : nodes) {
	os << "data: " << n.idx << " split: " << n.axis << "(" << n.left << ", " << n.right << ") @" << to_string(n.p) << endl;
    }
}
