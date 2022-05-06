/********
 * INCLUDE HEADER
 ***/
#include <iostream>
#include <chrono>
#include <vector>
#include <utility>
#include <algorithm>

#include <omp.h>

#include "common.hpp"
#include "mst.hpp"


/********
 * PRE-PROCESSOR
 ***/
#define KRUSKAL_THRESHOLD 1000
//#define DEBUG_SORTING


/********
 * GLOBAL MEM
 ***/
extern profile_times profiling;
extern algorithm_params params;
extern common_data common;
extern mst_data mst;
extern sample_sort_data ssort;
extern filter_kruskal_data fkruskal;

std::chrono::time_point<std::chrono::steady_clock> start;
std::chrono::time_point<std::chrono::steady_clock> end;
double sync_time = 0;

/********
* HELPER FUNCTIONS
***/
void print_edge_array(edge_w* edge_arr, const int n)
{
	for (int i = 0; i < n; ++i)
		std::cout << " " << edge_arr[i].rand_w << " ";
	std::cout << std::endl;
}


/********
* SORTING
***/
bool compare_edge_w(const edge_w& e1, const edge_w& e2) { return e1.rand_w < e2.rand_w; }

edge_w* parallel_sample_sort(edge_w* edge_src, edge_w* edge_dst, int n, int threads_used)
{	
	/*
	std::cout << "n = " << n << std::endl;
	std::cout << "threads_used = " << threads_used << std::endl;
	*/
	if (n <= 0)
		return edge_src;

	if (threads_used <= 1)
	{
		std::sort(edge_src, edge_src+n, compare_edge_w);
		return edge_src;
	}
	else if (threads_used*threads_used >= n)
		return parallel_sample_sort(edge_src, edge_dst, n, threads_used-1);
	else
	{
		// variables to check whether requested number of threads are available
		bool has_enough_threads = true;

		// variables for assigning bucket
		int dst_idx = 0;

		// get local samples
		const int num_samples_per_thread = threads_used-1;
		
		// sample_edges from ssort
		/* sample_edges = new edge_w[num_samples_per_thread*threads_used]; */
		edge_w* sample_edges = ssort.sample_edges; 

		// bucket_starts from ssort
		/* int* bucket_starts = new int[threads_used]; */
		int* bucket_starts = ssort.bucket_starts;

	
		// start sample sort
		#pragma omp parallel num_threads(threads_used) default(none) shared(edge_src, edge_dst, n, threads_used, has_enough_threads, dst_idx, num_samples_per_thread, sample_edges, bucket_starts, std::cout)
		{
			// get curr thread id
			const int tid = omp_get_thread_num();
			
			// check if there are enough threads
			#pragma omp single
			{
				int curr_threads_num = omp_get_num_threads();
				if (curr_threads_num < threads_used)
					has_enough_threads = false;
			}


			if (has_enough_threads)
			{
				int t_begin = -1;
				int t_end = -1;
				if (tid < threads_used)
				{
					// my local section on the original source array
					t_begin = (n / threads_used) * tid;
					t_end = (tid == threads_used-1)?(n-1):(t_begin+n/threads_used-1);	//// inclusive range
					const int local_list_len = t_end - t_begin + 1;	

					// sort my local list
					std::sort(edge_src+t_begin, edge_src+t_end+1, compare_edge_w);

					// extract samples from my local list into the shared sample_edges array
					const int gap_betw_samples = local_list_len / num_samples_per_thread;
					int start_sample_idx = tid * num_samples_per_thread;
					int end_sample_idx = (tid+1) * num_samples_per_thread;
					int sample_index = start_sample_idx;
					for (int i = t_begin; i < t_end && sample_index < end_sample_idx; i += gap_betw_samples)
					{
						sample_edges[sample_index] = edge_src[i];
						++sample_index;
					}
				}
				#pragma omp barrier

				#pragma omp single
				{
					// sort all samples
					std::sort(sample_edges, sample_edges+num_samples_per_thread*threads_used, compare_edge_w);
				}

				// use "threads_used - 1" number of splitters
				const int gap_betw_splitters = (num_samples_per_thread*threads_used) / (threads_used - 1);

				// organize the elements based on the global splitters
				int curr_src_idx = t_begin;
				int sample_index = 0;
				for (int i = 0; i < threads_used; ++i)
				{
					#pragma omp single
					{
						// start index for each bucket
						bucket_starts[i] = dst_idx;
					}
					
					if (tid < threads_used)
					{
						if (i == threads_used-1)
						{
							
							// put anything larger than the last splitter into the last bucket
							while (curr_src_idx <= t_end)
							{
								int idx_to_store;
								
								#pragma omp atomic capture
								idx_to_store = dst_idx++;
								
								edge_dst[idx_to_store] = edge_src[curr_src_idx];
								++curr_src_idx;
							}
						}
						else
						{
							int curr_splitter = sample_edges[sample_index].rand_w;

							// put all elements smaller than current splitter into the current bucket
							while (curr_src_idx <= t_end && edge_src[curr_src_idx].rand_w < curr_splitter)
							{
								int idx_to_store;
								
								#pragma omp atomic capture
								idx_to_store = dst_idx++;
								
								edge_dst[idx_to_store] = edge_src[curr_src_idx];
								++curr_src_idx;
							}
						}

						sample_index += gap_betw_splitters;
					}

					#pragma omp barrier
				}

				if (tid < threads_used)
				{
					// sort each bucket individually
					int my_bucket_start = bucket_starts[tid];
					int my_bucket_len = (tid == threads_used-1)?(n-my_bucket_start):(bucket_starts[tid+1]-bucket_starts[tid]);
					
					if (my_bucket_len > 0)
						std::sort(edge_dst+my_bucket_start, edge_dst+my_bucket_start+my_bucket_len, compare_edge_w);
				}
			}
		}

		// after the parallel section
		if (has_enough_threads)
			return edge_dst;
		else
		{
			std::cout << "Parallel Sample Sort: Insufficient Threads" << std::endl;
			return parallel_sample_sort(edge_src, edge_dst, n, threads_used-1);
		}
	}
}


/********
* FILTER KRUSKAL ALGORITHM
***/
int kruskal(UndirectedGraph_t& graph, UnionFind& uf_mst, const int remaining_edges, 
			edge_w* edge_src, edge_w* edge_dst, const int n)
{
	// sort the edge based on random weights
	edge_w* sorted_edges = parallel_sample_sort(edge_src, edge_dst, n, ssort.thread_nums);
	
	// finding the MST
	int numEdgesInMST = 0;
	for (int i = 0; i < n; ++i)
	{
		if (numEdgesInMST >= remaining_edges)
			break;
		
		int edgeId = sorted_edges[i].edge_id;
		int v1 = graph.edges[edgeId].v1;
		int v2 = graph.edges[edgeId].v2;
		int w = graph.edges[edgeId].w;
		if (uf_mst.union_set(v1, v2, w))
		{
			numEdgesInMST++;
			mst.inMST[edgeId] = true;
		}
	}
	return numEdgesInMST;
}

int pick_pivot(edge_w* e, int n)
{
	const int rand_num = (*(fkruskal.rand_int))(fkruskal.generator);
	const int pivot_idx = rand_num % n;
	return e[pivot_idx].rand_w;
}

std::pair<int, int> partition(edge_w* edge_src, const int n,
							  const int pivot, edge_w* l_partition, edge_w* h_partition)
{
	int l_idx = 0;
	int h_idx = 0;
	
	/*
	#pragma omp parallel for num_threads(68) default(none) shared(edge_src, n, pivot, l_partition, h_partition, l_idx, h_idx)
	for (int i = 0; i < n; ++i)
	{
		int idx_to_store;
		
		if (edge_src[i].rand_w <= pivot)
		{
			#pragma omp atomic capture
			idx_to_store = l_idx++;						

			l_partition[idx_to_store] = edge_src[i];
		}
		else
		{
			#pragma omp atomic capture
			idx_to_store = h_idx++;						

			h_partition[idx_to_store] = edge_src[i];
		}
	}
	*/

	if (n >= 1000)
	{
		#pragma omp parallel for default(none) shared(edge_src, n, pivot, l_partition, h_partition, l_idx, h_idx) schedule(static)
		for (int i = 0; i < n; ++i)
		{
			int idx_to_store;
			
			if (edge_src[i].rand_w <= pivot)
			{
				#pragma omp atomic capture
				idx_to_store = l_idx++;						

				l_partition[idx_to_store] = edge_src[i];
			}
			else
			{
				#pragma omp atomic capture
				idx_to_store = h_idx++;						

				h_partition[idx_to_store] = edge_src[i];
			}
		}
	}
	else
	{
		for (int i = 0; i < n; ++i)
		{
			int idx_to_store;
			
			if (edge_src[i].rand_w <= pivot)
				l_partition[l_idx++] = edge_src[i];
			else
				h_partition[h_idx++] = edge_src[i];
		}
	}
	
	return std::make_pair(l_idx, h_idx);
}

int filter(UndirectedGraph_t& graph, UnionFind& uf_mst,
		   edge_w* edge_src, edge_w* edge_dst, const int n)
{
	int filter_len = 0;

	/*	
	#pragma omp parallel for num_threads(68) default(none) shared(graph, uf_mst, edge_src, edge_dst, n, filter_len)
	for (int i = 0; i < n; ++i)
	{
		const int edgeId = edge_src[i].edge_id;
		const int v1 = graph.edges[edgeId].v1;
		const int v2 = graph.edges[edgeId].v2;
		if (uf_mst.parallel_find(v1) != uf_mst.parallel_find(v2))
		{
			int idx_to_store;
			
			#pragma omp atomic capture
			idx_to_store = filter_len++;

			edge_dst[idx_to_store] = edge_src[i];
		}
	}
	*/

	if (n >= 1000)
	{
		#pragma omp parallel for num_threads(68) default(none) shared(graph, uf_mst, edge_src, edge_dst, n, filter_len)
		for (int i = 0; i < n; ++i)
		{
			const int edgeId = edge_src[i].edge_id;
			const int v1 = graph.edges[edgeId].v1;
			const int v2 = graph.edges[edgeId].v2;
			if (uf_mst.parallel_find(v1) != uf_mst.parallel_find(v2))
			{
				int idx_to_store;
				
				#pragma omp atomic capture
				idx_to_store = filter_len++;

				edge_dst[idx_to_store] = edge_src[i];
			}
		}
	}
	else
	{
		for (int i = 0; i < n; ++i)
		{
			const int edgeId = edge_src[i].edge_id;
			const int v1 = graph.edges[edgeId].v1;
			const int v2 = graph.edges[edgeId].v2;
			if (uf_mst.find(v1) != uf_mst.find(v2))
				edge_dst[filter_len++] = edge_src[i];
		}
	}

	return filter_len;
}

int filter_kruskal(UndirectedGraph_t& graph, UnionFind& uf_mst, const int remaining_edges,
				   edge_w* edge_src, edge_w* edge_dst, const int n)
{
	if (n < KRUSKAL_THRESHOLD)
		return kruskal(graph, uf_mst, remaining_edges, edge_src, edge_dst, n);
	else
	{
		/* std::cout << "recurse on filter_kruskal" << std::endl; */
		
		// pick a random pivot from source array
		const int pivot = pick_pivot(edge_src, n);
		
		/* std::cout << "finshied picking pivot" << std::endl; */

		// partitioning using the pivot
		edge_w* l_partition = new edge_w[n];
		edge_w* h_partition = new edge_w[n];
		
		
		std::pair<int, int> partition_lens = partition(edge_src, n, pivot, l_partition, h_partition);


		const int l_len = partition_lens.first;
		const int h_len = partition_lens.second;

		/* std::cout << "finished partitioning - l_len = " << l_len << " ; " << "h_len = " << h_len << std::endl; */

		// filtering
		const int a_len = filter_kruskal(graph, uf_mst, remaining_edges, l_partition, edge_dst, l_len);
		
		
		const int filter_len = filter(graph, uf_mst, h_partition, l_partition, h_len);	// using the l_partition as a buffer for storing the filtered edges
		

		const int b_len = filter_kruskal(graph, uf_mst, remaining_edges-a_len, l_partition, edge_dst+a_len, filter_len);

		/* std::cout << "starting to release mem" << std::endl; */

		// release memory
		delete[] l_partition;
		delete[] h_partition;

		return a_len + b_len;
	}
}


/********
* SOURCE
***/
void findMST(UndirectedGraph_t& graph, UnionFind& uf_mst)
{
	profiling.clear_start = std::chrono::steady_clock::now();
	// reset union find tree
	uf_mst.clear();
	profiling.clear_end = std::chrono::steady_clock::now();


	profiling.f_kruskal_start = std::chrono::steady_clock::now();
	// the filter kruskal algorithm
	filter_kruskal(graph, uf_mst, params.V-1, mst.edge_mst_buf, mst.edge_mst_queue, params.E);
	profiling.f_kruskal_end = std::chrono::steady_clock::now();


	/*
	#ifdef DEBUG_SORTING
	std::cout << "<<<<<<<<<<<<<<<<<<<<" << std::endl;
	print_edge_array(mst.edge_mst_buf, params.E);
	#endif

	profiling.sort_start = std::chrono::steady_clock::now();
	// sort the edge based on random weights
	edge_w* sorted_edges = parallel_sample_sort(mst.edge_mst_buf, mst.edge_mst_queue, params.E, ssort.thread_nums);
	profiling.sort_end = std::chrono::steady_clock::now();

	#ifdef DEBUG_SORTING
	std::cout << ">>>>>>>>>>>>>>>>>>>>" << std::endl;
	print_edge_array(sorted_edges, params.E);	
	#endif

	
	profiling.pq_start = std::chrono::steady_clock::now();
	// finding the MST
	int numEdgesInMST = 0;
	for (int i = 0; i < params.E; ++i)
	{
		if (numEdgesInMST >= params.V-1)
			break;
		
		int edgeId = sorted_edges[i].edge_id;
		int v1 = graph.edges[edgeId].v1;
		int v2 = graph.edges[edgeId].v2;
		int w = graph.edges[edgeId].w;
		if (uf_mst.union_set(v1, v2, w))
		{
			numEdgesInMST++;
			mst.inMST[edgeId] = true;
		}
	}
	profiling.pq_end = std::chrono::steady_clock::now();
	*/
}
