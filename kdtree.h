#ifndef KDTREE_H_
#define KDTREE_H_

#include "common.h"

struct KDNode {
    vec2 p;
    int idx; /// the data
    int left, right, axis;
    KDNode(vec2 p, int idx)
	: p(p), idx(idx), left(-1), right(-1), axis(-1) {};
    
};

class KDTree {
public:
    void insert(const KDNode& n);
    const KDNode& findNearest(const vec2& p) const;
    const KDNode& operator[] (size_t idx) const;
    void nnsearch(const vec2& p, int root, float &r2, int& best) const;
    void clear();
    void dumpNodes(ostream& os);
private:
    void insertLastInTree(KDNode& r);
    void insertLeft(KDNode& r);
    void insertRight(KDNode& r);
    bool leftOf(const KDNode& r, const KDNode& n) const;
    bool leftOf(const KDNode& r, const vec2& p) const;

    vector<KDNode> nodes;
};


#endif /* !KDTREE_H_ */
