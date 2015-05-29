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
    KDTree();
    void build();
    void insert(const KDNode& n);
    const KDNode& findNearest(const vec2& p) const;
    KDNode& operator[] (size_t idx);
    void nnsearch(const vec2& p, int root, float &r2, int& best) const;
    void clear();
    void rebuild();
    void dumpNodes(ostream& os);
    size_t getSize() const;
private:
    void insertLastInTree(KDNode& r);
    void insertLeft(KDNode& r);
    void insertRight(KDNode& r);
    bool leftOf(const KDNode& r, const KDNode& n) const;
    bool leftOf(const KDNode& r, const vec2& p) const;

    vector<KDNode> nodes;
    size_t size;
};


#endif /* !KDTREE_H_ */
