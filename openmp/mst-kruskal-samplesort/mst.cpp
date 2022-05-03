/********
 * INCLUDE HEADER
 ***/
#include <iostream>
#include <chrono>

#include <omp.h>

#include "mst.hpp"


/********
* SORTING
***/
int binarySearch(edge_w* e, int n, int val)
{
	int start = 0;
	int end = n-1;
	while (start <= end)
	{
		int mid = (start + end) / 2;
		if (e[mid].rand_w == val)
			return mid;
		else if (e[mid].rand_w < val)
			start = mid+1;
		else
			end = mid-1;
	}
	return end;
}

void parallel_merge(edge_w* A, int A_len, edge_w* B, int B_len, edge_w* C)
{
	if (A_len < B_len)
		parallel_merge(B, B_len, A, A_len, C);
	else if (A_len <= 0)
		return;
	else
	{
		int mid_a = A_len / 2;
		int cut_b = binarySearch(B, B_len, A[mid_a].rand_w);
		C[mid_a+cut_b+1] = A[mid_a];
		
		int totalLen = A_len + B_len;

		#pragma omp task if (totalLen > 700)
		parallel_merge(A, mid_a, B, cut_b+1, C);

		#pragma omp task if (totalLen > 700)
		parallel_merge(A+mid_a+1, A_len-mid_a-1, B+cut_b+1, B_len-cut_b-1, C+mid_a+cut_b+2);
	
		#pragma omp taskwait
	}
}

void parallel_merge_sort(edge_w* edge_src, edge_w* edge_dst, int n)
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

		// create buf
		edge_w* C = new edge_w[n];
		
		// recurse on each half
		#pragma omp task if (n > 700)
		parallel_merge_sort(first_half, C, first_len); 
		
		#pragma omp task if (n > 700)
		parallel_merge_sort(second_half, C+mid, second_len);
		
		#pragma omp taskwait

		// merge the two halves
		parallel_merge(C, first_len, C+mid, second_len, edge_dst); 
		
		delete[] C;
	}
}


/********
* SOURCE
***/
// for profiling
extern std::chrono::time_point<std::chrono::steady_clock> clear_start;
extern std::chrono::time_point<std::chrono::steady_clock> clear_end;
extern std::chrono::time_point<std::chrono::steady_clock> sort_start;
extern std::chrono::time_point<std::chrono::steady_clock> sort_end;
extern std::chrono::time_point<std::chrono::steady_clock> pq_start;
extern std::chrono::time_point<std::chrono::steady_clock> pq_end;

// global variable
extern int E;
extern int V;
extern bool* inMST;

void findMST(UndirectedGraph_t& graph, UnionFind& uf_mst, edge_w* edge_mst_buf, edge_w* edge_mst_queue)
{
	clear_start = std::chrono::steady_clock::now();
	// reset union find tree
	uf_mst.clear();
	clear_end = std::chrono::steady_clock::now();

	sort_start = std::chrono::steady_clock::now();
	// sort the edge based on random weights
	#pragma omp parallel num_threads(68) default(none) shared(edge_mst_buf, edge_mst_queue, E)
	{
		#pragma omp single
		parallel_merge_sort(edge_mst_buf, edge_mst_queue, E);
	}
	sort_end = std::chrono::steady_clock::now();
	
	pq_start = std::chrono::steady_clock::now();
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
		{
			numEdgesInMST++;
			inMST[edgeId] = true;
		}
	}
	pq_end = std::chrono::steady_clock::now();
}
