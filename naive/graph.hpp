/********
 * INCLUDE HEADER
 ***/
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


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
		const uint64_t v1;		// the first endpoint (smaller node id)
		const uint64_t v2;		// the second endpoint (larger node id)
		unsigned int w;			// the weight of the edge
		const bool isFreeLoop;	// indicates whether this edge is a self loop	

		// Constructor
		Edge_t() = delete;
		
		Edge_t(uint64_t v1, uint64_t v2, unsigned int w)
			:v1{(v1>v2)?v2:v1}, v2{(v1>v2)?v1:v2}, w{w}, isFreeLoop{v1 == v2}
		{
			if (isFreeLoop && w == 0)
				throw EdgeException("Detected a free loop with negative weight");
		}

		// Operator Overloading
		friend std::ostream& operator<<(std::ostream& out, const Edge_t& edge)
		{
			out << edge.v1 << " " << edge.v2 << " " << edge.w;
			return out;
		}	
};

class Graph_t {
	public:
		// Members
		uint64_t V;			// total number of nodes in this graph
		std::vector<Edge_t> edges;	// vector for edges

		// Constructor
		Graph_t() = delete;

		Graph_t(const uint64_t node_nums)
			:V{node_nums}
		{}
		
		Graph_t(const char* filename)
		{
			// open the file
			std::ifstream graph_txt(filename);
			if (!graph_txt.good()) throw GraphException("Failed to open graph text file");

	
			// read the number of nodes
			if (!(graph_txt >> V)) throw GraphException("Graph text file has incorrect format (nodeNums)");

			// read the number edges
			uint64_t numEdges;
			if (!(graph_txt >> numEdges)) throw GraphException("Graph text file has incorrect format (edgeNums)");

			// read all the edges
			uint64_t v1;
			uint64_t v2;
			int w;
			for (uint64_t i = 0; i < numEdges; ++i)
			{
				if (!(graph_txt >> v1 >> v2 >> w)) throw GraphException("Graph text file has incorrect format (edges)");
				insertEdge(v1, v2, w);
			}

			// close the file
			graph_txt.close();
		}


		// Methods
		void insertEdge(uint64_t v1, uint64_t v2, unsigned int w)
		{
			edges.push_back(Edge_t(v1, v2, w));
		}

		// Operator Overloading
		friend std::ostream& operator<<(std::ostream& out, const Graph_t& graph)
		{
			for (auto& edge : graph.edges)
			{
				out << edge << std::endl;
			}
			return out;
		}
};
