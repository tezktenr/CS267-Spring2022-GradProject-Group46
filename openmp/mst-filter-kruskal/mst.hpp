#ifndef __MST_HPP__
#define __MST_HPP__

/********
 * INCLUDE HEADER
 ***/
#include <iostream>

#include "graph.hpp"
#include "union_find.hpp"


/********
 * DATA STRUCT
 ***/
struct edge_w {
	int edge_id;
	int rand_w;
};

struct bucket {
	int currId;
};


/********
 * MST FUNCTIONS
 ***/
void findMST(UndirectedGraph_t& graph, UnionFind& uf_mst);


#endif
