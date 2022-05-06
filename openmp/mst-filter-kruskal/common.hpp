#ifndef __COMMON_HPP__
#define __COMMON_HPP__

/********
 * INCLUDE HEADER
 ***/
#include <iostream>
#include <chrono>
#include <random>

#include "mst.hpp"


/********
 * PRE-PROCESSOR
 ***/


/********
 * COMMON DATA STRUCTURE
 ***/
struct profile_times {
	std::chrono::time_point<std::chrono::steady_clock> gen_random_start;
	std::chrono::time_point<std::chrono::steady_clock> gen_random_end;
	std::chrono::time_point<std::chrono::steady_clock> find_mst_start;
	std::chrono::time_point<std::chrono::steady_clock> find_mst_end;
	std::chrono::time_point<std::chrono::steady_clock> clear_start;
	std::chrono::time_point<std::chrono::steady_clock> clear_end;
	//std::chrono::time_point<std::chrono::steady_clock> sort_start;
	//std::chrono::time_point<std::chrono::steady_clock> sort_end;
	//std::chrono::time_point<std::chrono::steady_clock> pq_start;
	//std::chrono::time_point<std::chrono::steady_clock> pq_end;
	std::chrono::time_point<std::chrono::steady_clock> f_kruskal_start;
	std::chrono::time_point<std::chrono::steady_clock> f_kruskal_end;
	std::chrono::time_point<std::chrono::steady_clock> compress_start;
	std::chrono::time_point<std::chrono::steady_clock> compress_end;
	std::chrono::time_point<std::chrono::steady_clock> gen_tree_start;
	std::chrono::time_point<std::chrono::steady_clock> gen_tree_end;
	std::chrono::time_point<std::chrono::steady_clock> balance_start;
	std::chrono::time_point<std::chrono::steady_clock> balance_end;
	std::chrono::time_point<std::chrono::steady_clock> start_time;
	std::chrono::time_point<std::chrono::steady_clock> end_time;

	double time(std::chrono::time_point<std::chrono::steady_clock> start,
				std::chrono::time_point<std::chrono::steady_clock> end)
	{
		return ((std::chrono::duration<double>)(end - start)).count();
	}
};


struct algorithm_params {
	// total number of nodes
	int V;

	// total number of edges
	int E;	

	// iterations
	int iterations;

	// the seed
	unsigned int seed;
};


struct common_data {
	// the updated edge weights (1 or -1) after balancing for an iteration
	int* balancedWeights;


	// -----------------------------
	// destructor for memory release
	// -----------------------------
	~common_data() 
	{
		if (balancedWeights != nullptr)
			delete[] balancedWeights;
	}
};


struct mst_data {
	// indicate whether an edge is selected to be in the MST for an iteration
	bool* inMST;

	// buffer for edges random weights used in MST
	edge_w* edge_mst_buf;

	// sorted edges random weights used in MST (if NOT sorted in place)
	edge_w* edge_mst_queue;
	

	// -----------------------------
	// destructor for memory release
	// -----------------------------
	~mst_data() 
	{
		if (inMST != nullptr)
			delete[] inMST;
		if (edge_mst_buf != nullptr)
			delete[] edge_mst_buf;
		if (edge_mst_queue != nullptr)
			delete[] edge_mst_queue;
	}
};


struct sample_sort_data {
	// number of threads intended to be used for sorting
	int thread_nums;

	// sample edges list used during sample sort
	edge_w* sample_edges;

	// the start index for each bucket of sample sort
	int* bucket_starts;
	

	// -----------------------------
	// destructor for memory release
	// -----------------------------
	~sample_sort_data() 
	{
		if (sample_edges != nullptr)
			delete[] sample_edges;
		if (bucket_starts != nullptr)
			delete[] bucket_starts;
	}
};


struct filter_kruskal_data {
	// source of randomness
	std::default_random_engine generator;

	// uniform int generator for pivot selection
	std::uniform_int_distribution<int>* rand_int;

	// -----------------------------
	// destructor for memory release
	// -----------------------------
	~filter_kruskal_data() 
	{}
};



#endif
