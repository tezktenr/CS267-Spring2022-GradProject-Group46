/********
 * INCLUDE HEADER
 ***/
#include <iostream>
#include "mst.hpp"


/********
* SORTING
***/
void serial_merge(edge_w* A, int A_len, edge_w* B, int B_len, edge_w* C)
{
	int a_idx = 0;
	int b_idx = 0;
	for (int i = 0; i < A_len+B_len; ++i)
	{
		if (b_idx >= B_len || (a_idx < A_len && A[a_idx].rand_w <= B[b_idx].rand_w))
			C[i] = A[a_idx++];
		else
			C[i] = B[b_idx++];
	}
}

void serial_merge_sort(edge_w* edge_src, edge_w* edge_dst, int n)
{
	if (n == 1)
		edge_dst[0] = edge_src[0];
	else if (n > 1)
	{
		// calculate the middle index
		int mid = n / 2;

		// divide array into two halves
		edge_w* first_half = edge_src;
		int first_len = mid;
		edge_w* second_half = edge_src + mid;
		int second_len = n - mid;

		// recurse on each half
		serial_merge_sort(first_half, edge_dst, first_len);
		serial_merge_sort(second_half, edge_dst+mid, second_len);
		
		// merge the two halves
		serial_merge(first_half, first_len, second_half, second_len, edge_dst);

		// save to src (necessary step for correctness)
		for (int i = 0; i < n; ++i)
			edge_src[i] = edge_dst[i];
	}
}


/********
* SOURCE
***/
extern int E;
extern int V;
extern bool* inMST;

void findMST(UndirectedGraph_t& graph, UnionFind& uf_mst, edge_w* edge_mst_buf, edge_w* edge_mst_queue)
{
	// reset union find tree
	uf_mst.clear();

	// sort the edge based on random weights
	serial_merge_sort(edge_mst_buf, edge_mst_queue, E);

	// finding the MST
	int numEdgesInMST = 0;
	for (int i = 0; i < E; ++i)
	{
		if (numEdgesInMST >= V-1)
			break;
		
		int edgeId = edge_mst_queue[i].edge_id;
		int v1 = graph.edges[edgeId].v1;
		int v2 = graph.edges[edgeId].v2;
		int w = graph.edges[edgeId].w;
		if (uf_mst.union_set(v1, v2, w))
			inMST[edgeId] = true;
	}

}
