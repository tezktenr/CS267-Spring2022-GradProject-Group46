#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

/********
 * INCLUDE HEADER
 ***/
#include <cstdio>
#include <iostream>
#include <fstream>
#include <exception>
#include <algorithm>
#include <utility>
#include <tuple>
#include <string>
#include <set>
#include <vector>
#include <unordered_map>


/********
 * PRE-PROCESSOR
 ***/
//#define DEBUG_GRAPH


/********
 * GRAPH EXCEPTION
 ***/
class EdgeException : public std::runtime_error {
	public:
		EdgeException(const char* msg) : std::runtime_error(msg)
		{}
};

class GraphException : public std::runtime_error {
	public:
		GraphException(const char* msg) : std::runtime_error(msg)
		{}
};


/********
 * GRAPH DATA STRUCTURE
 ***/
class Edge_t {
	public:
		// Members
		const int v1;		// the first endpoint (smaller node id)
		const int v2;		// the second endpoint (larger node id)
		int w;				// the weight of the edge

		// Constructor
		Edge_t() = delete;
		
		Edge_t(int v1, int v2, int w)
			:v1{v1}, v2{v2}, w{w}
		{
			if (v1 == v2) throw EdgeException("Detected a free loop that got inserted");
			if (w != -1 && w != 1) throw EdgeException("Detected an edge with invalid weight");
		}

		// Operator Overloading
		friend std::ostream& operator<<(std::ostream& out, const Edge_t& edge)
		{
			out << edge.v1 << " " << edge.v2 << " " << edge.w;
			return out;
		}
};

class UndirectedGraph_t {
	public:

		// Total number of nodes in this graph
		uint64_t V;

		// Store all the edges in this graph
		std::vector<Edge_t> edges;

		// Store the mapping of nodes in this graph 
		// Example: (Node 1000 -> 0, Node 1001 -> 1, Node 1002 -> 2; V = 3)
		std::unordered_map<int, int> nodes_map;

		// Constructor
		UndirectedGraph_t() = delete;
		
		UndirectedGraph_t(const char* filename)
			:V{0}
		{
			// reading the graph from CSV
			readGraphCSV(filename);

			// choosing the largest CC
			//...
		}

		// Operator Overloading
		friend std::ostream& operator<<(std::ostream& out, const UndirectedGraph_t& graph)
		{
			for (auto& edge : graph.edges)
			{
				out << edge << std::endl;
			}
			return out;
		}

	private:
	
		void readGraphCSV(const char* filename)
		{
			// open the file
			std::ifstream graph_csv(filename);
			if (!graph_csv.good()) throw GraphException("Failed to open graph csv file");

			// Skip the first line is "From Node ID, To Node ID, Edge Weight"
			std::string line;
			std::getline(graph_csv, line);
			if (line != "From Node ID, To Node ID, Edge Weight") throw GraphException("Incorrect format of graph csv file (first line)");

			// Error handling
			unsigned int self_loops = 0, duplicates = 0, inconsistent = 0;
			std::set<std::pair<int, int>> edge_set1;		// for detecting duplicate edges
			std::set<std::tuple<int, int, int>> edge_set2;	// for detecting inconsistent edges

			// read all the edges
			int v1;
			int v2;
			int w;
			while (std::getline(graph_csv, line))
			{
				// read v1, v2, w
				if (std::sscanf(line.c_str(), "%d,%d,%d", &v1, &v2, &w) != 3) 
					break;

				// make v1 smaller than v2
				if (v1 > v2)
					std::swap(v1, v2);

				// for error handling
				std::pair<int, int> edge_pair = std::make_pair(v1, v2);
				std::tuple<int, int, int> edge_tuple = std::make_tuple(v1, v2, w);

				// check if the edge is duplicated in the set
				if (edge_set1.find(edge_pair) != edge_set1.end())
				{
					if (edge_set2.find(edge_tuple) != edge_set2.end())
						++inconsistent;
					else
						++duplicates;
				}

				// insert edge into the graph
				if (v1 != v2)
				{
					if (!hasNodeId(v1))
						nodes_map[v1] = V++;

					if (!hasNodeId(v2))
						nodes_map[v2] = V++;

					insertEdge(nodes_map[v1], nodes_map[v2], w);
					
					// insert the edge into the set for error checking
					edge_set1.insert(edge_pair);
					edge_set2.insert(edge_tuple);
				}
				else
					++self_loops;
			}

			// close the file
			graph_csv.close();

			#ifdef DEBUG_GRAPH
			std::cout << "Graph read complete!" << std::endl;
			std::cout << "Read from file: " << V << " nodes and " << edges.size() << " edges" << std::endl;
			std::cout << " - skipped " << self_loops << " self loops" << std::endl;
			std::cout << " - skipped " << duplicates << " duplicate edges" << std::endl;
			std::cout << " - skipped " << inconsistent << " inconsistent edges" << std::endl;
			std::cout << std::endl;
			#endif 
		}
	
		void insertEdge(uint64_t v1, uint64_t v2, unsigned int w)
		{
			edges.push_back(Edge_t(v1, v2, w));
		}

		bool hasNodeId(const int node_id)
		{
			return nodes_map.find(node_id) != nodes_map.end(); 
		}
		
};


#endif
