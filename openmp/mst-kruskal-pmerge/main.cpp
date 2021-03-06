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

#include "mst.hpp"
#include "graph.hpp"
#include "union_find.hpp"


/********
 * PRE-PROCESSOR
 ***/
#define IMPL_NAME "Openmp - Kruskal PMerge"

#define PROFILING
//#define TEST_CORRECTNESS
//#define DEBUG_MAIN


/********
 * PROFILING
 ***/
std::chrono::time_point<std::chrono::steady_clock> gen_random_start;
std::chrono::time_point<std::chrono::steady_clock> gen_random_end;
std::chrono::time_point<std::chrono::steady_clock> find_mst_start;
std::chrono::time_point<std::chrono::steady_clock> find_mst_end;
std::chrono::time_point<std::chrono::steady_clock> clear_start;
std::chrono::time_point<std::chrono::steady_clock> clear_end;
std::chrono::time_point<std::chrono::steady_clock> sort_start;
std::chrono::time_point<std::chrono::steady_clock> sort_end;
std::chrono::time_point<std::chrono::steady_clock> pq_start;
std::chrono::time_point<std::chrono::steady_clock> pq_end;
std::chrono::time_point<std::chrono::steady_clock> compress_start;
std::chrono::time_point<std::chrono::steady_clock> compress_end;
std::chrono::time_point<std::chrono::steady_clock> gen_tree_start;
std::chrono::time_point<std::chrono::steady_clock> gen_tree_end;
std::chrono::time_point<std::chrono::steady_clock> balance_start;
std::chrono::time_point<std::chrono::steady_clock> balance_end;


/********
 * GLOBAL MEM
 ***/
// seed
unsigned int seed;

// number of edges
int E = 0;

// number of nodes
int V = 0;

// number of spanning trees to generate
int iterations = 0;

// data structure for MST
edge_w* edge_mst_buf = nullptr;		// buffer for initializing random weights for MST

edge_w* edge_mst_queue = nullptr;	// the queue of edges for MST based on random weights

bool* inMST = nullptr;				// storing whether edge is in MST for each iteration

int* balancedWeights = nullptr;		// storing weights of each edge after balance for each iteration


/********
* SOURCE
***/
void generateSpanningTree(UndirectedGraph_t& graph, UnionFind& uf_mst)
{
	// generate the random weights for each edge for MST
	gen_random_start = std::chrono::steady_clock::now();
	#ifdef TEST_CORRECTNESS
	for (int edgeId = 0; edgeId < E; ++edgeId)	
	{
		edge_mst_buf[edgeId].edge_id = edgeId;
		int rw = rand_r(&seed);
		edge_mst_buf[edgeId].rand_w = rw;
		inMST[edgeId] = false;
	}
	#else
	#pragma omp parallel num_threads(68) default(none) shared(seed, edge_mst_buf, inMST, E)
	{
		unsigned int randseed = seed * omp_get_thread_num();
		#pragma omp for schedule(static)
		for (int edgeId = 0; edgeId < E; ++edgeId)	
		{
			edge_mst_buf[edgeId].edge_id = edgeId;
			edge_mst_buf[edgeId].rand_w = rand_r(&randseed);
			inMST[edgeId] = false;
		}
	}
	#endif
	gen_random_end = std::chrono::steady_clock::now();


	// find the mst
	find_mst_start = std::chrono::steady_clock::now();
	findMST(graph, uf_mst, edge_mst_buf, edge_mst_queue);
	find_mst_end = std::chrono::steady_clock::now();


	compress_start = std::chrono::steady_clock::now();
	for (int nodeId = 0; nodeId < V; ++nodeId)
		uf_mst.find(nodeId);
	compress_end = std::chrono::steady_clock::now();
}

void balanceSpanningTree(UndirectedGraph_t& graph, UnionFind& uf_mst)
{
	#pragma omp parallel for default(none) shared(E, graph, uf_mst, inMST, balancedWeights)
	for (int edgeId = 0; edgeId < E; ++edgeId)
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
}


/********
* HELPER FUNCTIONS
***/
void print_balanced_state_MST(UndirectedGraph_t& graph)
{
	for (int edgeId = 0; edgeId < E; ++edgeId)
	{
		const int v1 = graph.edges[edgeId].v1;
		const int v2 = graph.edges[edgeId].v2;
		const int w = graph.edges[edgeId].w;
		std::cout << v1 << "," << v2 << "," << w << " [" << inMST[edgeId] << " ; " << balancedWeights[edgeId] << "]" << std::endl;
	}
	std::cout << std::endl;
}

void save(std::ofstream& file, const int iteration, UndirectedGraph_t& graph, int* balancedWeights)
{
	file << "iteration = " << iteration << std::endl;
	for (int edgeId = 0; edgeId < E; ++edgeId)
	{
		const int v1 = graph.edges[edgeId].v1;
		const int v2 = graph.edges[edgeId].v2;
		const int w = graph.edges[edgeId].w;
		file << v1 << "," << v2 << "," << w << " [" << inMST[edgeId] << " ; " << balancedWeights[edgeId] << "]" << std::endl;
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
	E = graph.edges.size();
	V = graph.V;
	#ifdef DEBUG_MAIN
	std::cout << graph << std::endl;
	#endif

	// Parse -o: correctness output file
	char* save_filepath = find_string_option(argc, argv, "-o", nullptr);
	std::ofstream savefile(save_filepath);

	// Parse -n
	iterations = find_int_arg(argc, argv, "-n", 1);

	// construct data structure
	UnionFind uf_mst(V);
	edge_mst_buf = new edge_w[E];
	edge_mst_queue = new edge_w[E];
	inMST = new bool[E];
	balancedWeights = new int[E];

	// initialize data structure
	for (int i = 0; i < E; ++i)
	{
		edge_mst_buf[i].edge_id = i;
		edge_mst_buf[i].rand_w = 0;
	}

	// Parse -s
	seed = find_int_arg(argc, argv, "-s", time(nullptr));

	// Main Algorithms	
	auto start_time = std::chrono::steady_clock::now();


	for (int i = 0; i < iterations; ++i)
	{
		// generate spanning tree
		gen_tree_start = std::chrono::steady_clock::now();
		generateSpanningTree(graph, uf_mst);
		gen_tree_end = std::chrono::steady_clock::now();
		
		
		balance_start = std::chrono::steady_clock::now();
		balanceSpanningTree(graph, uf_mst);
		balance_end = std::chrono::steady_clock::now();


		#ifdef PROFILING
		std::cout << std::endl << "iterations = " << i << std::endl;
		std::cout << "gen tree: " << (double)(((std::chrono::duration<double>)(gen_tree_end - gen_tree_start)).count()) << "secs" << std::endl;
		std::cout << "+---> gen random numbers: " << (double)(((std::chrono::duration<double>)(gen_random_end - gen_random_start)).count()) << "secs" << std::endl;
		std::cout << "+---> find mst: " << (double)(((std::chrono::duration<double>)(find_mst_end - find_mst_start)).count()) << "secs" << std::endl;
		std::cout << "      +---> clear mst: " << (double)(((std::chrono::duration<double>)(clear_end - clear_start)).count()) << "secs" << std::endl;
		std::cout << "      +---> sort: " << (double)(((std::chrono::duration<double>)(sort_end - sort_start)).count()) << "secs" << std::endl;
		std::cout << "      +---> loop pq: " << (double)(((std::chrono::duration<double>)(pq_end - pq_start)).count()) << "secs" << std::endl;
		std::cout << "+---> path compress: " << (double)(((std::chrono::duration<double>)(compress_end - compress_start)).count()) << "secs" << std::endl;
		std::cout << "balance tree: " << (double)(((std::chrono::duration<double>)(balance_end - balance_start)).count()) << "secs" << std::endl;
		#endif

		#ifdef DEBUG_MAIN
		std::cout << std::endl;
		std::cout << "iterations = " << i << std::endl;
		for (int i = 0; i < V; ++i)
			std::cout << i << "->" << uf_mst.parent[i] << " = " << uf_mst.toParent[i] << std::endl;
		std::cout << std::endl;
		print_balanced_state_MST(graph);
		#endif		

		if (savefile.good())
			save(savefile, i, graph, balancedWeights);
	}


	auto end_time = std::chrono::steady_clock::now();
    double seconds = ((std::chrono::duration<double>)(end_time - start_time)).count();

    // Release Resource
	if (save_filepath != nullptr)
		savefile.close();
    
	// Finalize
	std::cout << "Runtime (" << IMPL_NAME << ") = " << seconds << " seconds" << std::endl;


	return 0;
}
