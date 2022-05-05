/********
 * INCLUDE HEADER
 ***/
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <utility>
#include <algorithm>
#include <chrono>

#include <omp.h>

#include "common.hpp"
#include "mst.hpp"
#include "graph.hpp"
#include "union_find.hpp"


/********
 * PRE-PROCESSOR
 ***/
#define IMPL_NAME "Openmp - Kruskal Parallel Sample Sort"

#define PROFILING
//#define TEST_CORRECTNESS
//#define DEBUG_MAIN


/********
 * GLOBAL MEM
 ***/
// For profiling the program runtime
profile_times profiling;

// Algorithm parameters
algorithm_params params;

// Common data structure
common_data common;

// MST data structure
mst_data mst;

// Sample sort data structure
sample_sort_data ssort;


/********
* SOURCE
***/
void generateSpanningTree(UndirectedGraph_t& graph, UnionFind& uf_mst)
{
	profiling.gen_tree_start = std::chrono::steady_clock::now();
	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

	
	// generate the random weights for each edge for MST
	profiling.gen_random_start = std::chrono::steady_clock::now();
	#ifdef TEST_CORRECTNESS
	for (int i = 0; i < params.E; ++i)	
	{
		int rw = rand_r(&(params.seed));
		
		mst.edge_mst_buf[i].edge_id = i;
		mst.edge_mst_buf[i].rand_w = rw;
		mst.inMST[i] = false;
	}
	#else
	#pragma omp parallel num_threads(68) default(none) shared(params, mst)
	{
		unsigned int threadseed = params.seed * omp_get_thread_num();
		#pragma omp for schedule(static)
		for (int i = 0; i < params.E; ++i)	
		{
			mst.edge_mst_buf[i].edge_id = i;
			mst.edge_mst_buf[i].rand_w = rand_r(&threadseed);
			mst.inMST[i] = false;
		}
	}
	#endif
	profiling.gen_random_end = std::chrono::steady_clock::now();


	// find the mst
	profiling.find_mst_start = std::chrono::steady_clock::now();
	findMST(graph, uf_mst);
	profiling.find_mst_end = std::chrono::steady_clock::now();


	// compress all paths one more time in the union find
	profiling.compress_start = std::chrono::steady_clock::now();
	for (int nodeId = 0; nodeId < params.V; ++nodeId)
		uf_mst.find(nodeId);
	profiling.compress_end = std::chrono::steady_clock::now();


	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	profiling.gen_tree_end = std::chrono::steady_clock::now();
}

void balanceSpanningTree(UndirectedGraph_t& graph, UnionFind& uf_mst)
{
	profiling.balance_start = std::chrono::steady_clock::now();
	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv


	// extract data structure needed
	int* balancedWeights = common.balancedWeights;	
	bool* inMST = mst.inMST;

	#pragma omp parallel for default(none) shared(graph, uf_mst, params, inMST, balancedWeights)
	for (int edgeId = 0; edgeId < params.E; ++edgeId)
	{
		int w = graph.edges[edgeId].w;
		if (inMST[edgeId])
			balancedWeights[edgeId] = w;
		else
		{
			int v1 = graph.edges[edgeId].v1;
			int v2 = graph.edges[edgeId].v2;
			balancedWeights[edgeId] = w * w * uf_mst.toParent[v1] * uf_mst.toParent[v2];
		}
	}

	
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	profiling.balance_end = std::chrono::steady_clock::now();
}


/********
* HELPER FUNCTIONS
***/
void print_balanced_state_MST(UndirectedGraph_t& graph)
{
	for (int edgeId = 0; edgeId < params.E; ++edgeId)
	{
		const int v1 = graph.edges[edgeId].v1;
		const int v2 = graph.edges[edgeId].v2;
		const int w = graph.edges[edgeId].w;
		std::cout << v1 << "," << v2 << "," << w << " [" << mst.inMST[edgeId] << " ; " << common.balancedWeights[edgeId] << "]" << std::endl;
	}
	std::cout << std::endl;
}

void save(std::ofstream& file, const int iteration, UndirectedGraph_t& graph)
{
	file << "iteration = " << iteration << std::endl;
	for (int edgeId = 0; edgeId < params.E; ++edgeId)
	{
		const int v1 = graph.edges[edgeId].v1;
		const int v2 = graph.edges[edgeId].v2;
		const int w = graph.edges[edgeId].w;
		file << v1 << "," << v2 << "," << w << " [" << mst.inMST[edgeId] << " ; " << common.balancedWeights[edgeId] << "]" << std::endl;
	}
}


/********
* COMMAND LINE ARG PARSING
***/
int find_arg_idx(int argc, char** argv, const char* option) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], option) == 0) {
            return i;
        }
    }
    return -1;
}

int find_int_arg(int argc, char** argv, const char* option, int default_value) {
    int iplace = find_arg_idx(argc, argv, option);

    if (iplace >= 0 && iplace < argc - 1) {
        return std::stoi(argv[iplace + 1]);
    }

    return default_value;
}

char* find_string_option(int argc, char** argv, const char* option, char* default_value) {
    int iplace = find_arg_idx(argc, argv, option);

    if (iplace >= 0 && iplace < argc - 1) {
        return argv[iplace + 1];
    }

    return default_value;
}


/********
 * MAIN
 ***/
int main(int argc, char** argv)
{
	// Help Info
    if (find_arg_idx(argc, argv, "-h") >= 0) {
        std::cout << "Check the Usage:" << std::endl;
        std::cout << "-h: see this help" << std::endl;
        std::cout << "-f <filename>: refer to the graph file name" << std::endl;
		std::cout << "-n <num>: number of spanning trees to generate" << std::endl;
		std::cout << "-o <filename>: refer to the output file for correctness" << std::endl;
		std::cout << "-s <seed_num>: the seed number" << std::endl;
        return 0;
    }

	// Parse -f: graph file name
	char* graph_filepath = find_string_option(argc, argv, "-f", nullptr);
	if (graph_filepath == nullptr)
	{
		std::cout << "Command Error: You must specify the graph file name via '-f'" << std::endl;
		return 1;
	}
	UndirectedGraph_t graph(graph_filepath);
	params.V = graph.V;
	params.E = graph.edges.size();
	#ifdef DEBUG_MAIN
	std::cout << graph << std::endl;
	#endif

	// Initialize common data structure
	common.balancedWeights = new int[params.E];

	// Initialize MST data structure
	UnionFind uf_mst(params.V);
	mst.inMST = new bool[params.E];
	mst.edge_mst_buf = new edge_w[params.E];
	mst.edge_mst_queue = new edge_w[params.E];

	// Initialize sample sort data structure
	#pragma omp parallel default(none) shared(params, ssort)
	{
		#pragma omp single
		{
			const int P = omp_get_num_threads();
			int thread_nums = P;
			while (thread_nums > 1 && thread_nums * thread_nums >= params.E)
				thread_nums--;
			ssort.thread_nums = thread_nums;
		}
	}
	ssort.sample_edges = new edge_w[(ssort.thread_nums-1)*ssort.thread_nums];
	ssort.bucket_starts = new int[ssort.thread_nums];

	// Parse -o: correctness output file
	char* save_filepath = find_string_option(argc, argv, "-o", nullptr);
	std::ofstream savefile(save_filepath);

	// Parse -n
	params.iterations = find_int_arg(argc, argv, "-n", 1);

	// Parse -s
	params.seed = find_int_arg(argc, argv, "-s", time(nullptr));


	// Main Algorithms	
	profiling.start_time = std::chrono::steady_clock::now();
	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	for (int i = 0; i < params.iterations; ++i)
	{
		// generate spanning tree
		generateSpanningTree(graph, uf_mst);
		
		// balance spanning tree	
		balanceSpanningTree(graph, uf_mst);
		
		// storing to output file if specified '-o'
		if (savefile.good())
			save(savefile, i, graph);


		#ifdef PROFILING
		std::cout << std::endl << "iterations = " << i << std::endl;
		std::cout << "gen tree: " << profiling.time(profiling.gen_tree_start, profiling.gen_tree_end) << "secs" << std::endl;
		std::cout << "+---> gen random numbers: " << profiling.time(profiling.gen_random_start, profiling.gen_random_end) << "secs" << std::endl;
		std::cout << "+---> find mst: " << profiling.time(profiling.find_mst_start, profiling.find_mst_end) << "secs" << std::endl;
		std::cout << "      +---> clear mst: " << profiling.time(profiling.clear_start, profiling.clear_end) << "secs" << std::endl;
		std::cout << "      +---> sort: " << profiling.time(profiling.sort_start, profiling.sort_end) << "secs" << std::endl;
		std::cout << "      +---> loop pq: " << profiling.time(profiling.pq_start, profiling.pq_end) << "secs" << std::endl;
		std::cout << "+---> path compress: " << profiling.time(profiling.compress_start, profiling.compress_end) << "secs" << std::endl;
		std::cout << "balance tree: " << profiling.time(profiling.balance_start, profiling.balance_end) << "secs" << std::endl;
		#endif

		#ifdef DEBUG_MAIN
		std::cout << std::endl;
		std::cout << "iterations = " << i << std::endl;
		for (int i = 0; i < params.V; ++i)
			std::cout << i << "->" << uf_mst.parent[i] << " = " << uf_mst.toParent[i] << std::endl;
		std::cout << std::endl;
		print_balanced_state_MST(graph);
		#endif		
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	profiling.end_time = std::chrono::steady_clock::now();


    // Release Resource
	if (save_filepath != nullptr)
		savefile.close();
	
	// Finalize
	double total_runtime = profiling.time(profiling.start_time, profiling.end_time);
	std::cout << "Runtime (" << IMPL_NAME << ") = " << total_runtime << " seconds" << std::endl;

	return 0;
}
