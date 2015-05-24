#include "common.h"

#include "kdtree.h"

#include <stdexcept>

#define ASSERT(COND, MSG) do {						\
	if (!(COND)) {							\
	    cerr << __FILE__ << ":" << __LINE__ << ":0 error Test Failed: " << MSG << endl; \
	    return false;						\
	} else {							\
	    cerr << "Test passed: " << MSG << endl;			\
	}								\
    } while(0);

bool instantiate() {
    KDTree t;
    t.insert(KDNode(vec2(0,0), 2));
    auto n = t.findNearest(vec2(0,0));
    ASSERT( n.idx = 42, "find the root");
    return true;
}

bool findEmpty() {
    KDTree t;
    try {
	auto n = t.findNearest(vec2(0));
    }  catch (const std::runtime_error& e) {
	ASSERT( true, "thows with not elements");
    }
    return true;
}

bool nodestart() {
    auto n = KDNode(vec2(0), 42);
    ASSERT(n.left == -1, "left  unitialised");
    ASSERT(n.right == -1, "right unitialised");
    ASSERT(n.axis == -1, "axis unitialised");

    KDTree t;
    t.insert(n);
    ASSERT(t[0].idx == 42, "data is here");
    ASSERT(t[0].left == -1, "left  unitialised");
    ASSERT(t[0].right == -1, "right unitialised");
    ASSERT(t[0].axis == 0, "axis splitts X");

    t.insert(KDNode(vec2(-1, 0), 2)); 	//  will to the left of the root
    ASSERT(t[0].left == 1, "left  is the new kid");
    ASSERT(t[0].right == -1, "right is untouched");

    t.insert(KDNode(vec2(1, 0), 2)); 	//  will to the right of the root
    ASSERT(t[0].right == 2, "right  is the new kid");
    
    ASSERT(t[1].axis == 1, "split on other axis");
    ASSERT(t[2].axis == 1, "split on other axis");

    return true;
}


bool insertSplitY() {
    KDTree t;
    t.insert(KDNode(vec2(0,0)  , 0));
    t.insert(KDNode(vec2(-1, 0), 1));
    t.insert(KDNode(vec2(-1, -1), 2));
    ASSERT(t[1].left == 2, "left of ");
    t.insert(KDNode(vec2(-1, 1), 3));
    ASSERT(t[1].right == 3, "right of ");

    ASSERT(t[2].axis == 0, "split on other axis");
    ASSERT(t[3].axis == 0, "split on other axis");
    
    return true;
}

bool simpleSearch() {
    KDTree t;
    t.insert(KDNode(vec2(0,0), 0));
    t.insert(KDNode(vec2(1,1), 1));
    auto n = t.findNearest(vec2(0,0));
    ASSERT( n.idx == 0, "search the tree");
    return true;
}

int main(int argc, char *argv[]) {
    return ! ( 
	instantiate()
	&& findEmpty()
	&& nodestart()
	&& insertSplitY()
	&& simpleSearch()
	);
}