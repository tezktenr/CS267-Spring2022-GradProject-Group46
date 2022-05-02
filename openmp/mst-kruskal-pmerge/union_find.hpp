#ifndef __UNION_FIND_HPP__
#define __UNION_FIND_HPP__

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
#define DEBUG_UNION_FIND


/********
 * UNINO FIND DATA STRUCTURE
 ***/
class UnionFind {
	public:
		// Members
		const int N;	// number of nodes in this set
		int* parent;	// parents of each node
		int* size;		// used for union by size
		int* rank;		// used for union by rank
		int* toParent;	// used for MST cycle processing, record the weight to parent of this node

		// Constructor & Destructor
		UnionFind() = delete;

		UnionFind(const int N)
			:N{N}
		{
			parent = new int[N];
			size = new int[N];
			rank = new int[N];
			toParent = new int[N];
			clear();
		}

		~UnionFind()
		{
			delete[] parent;
			delete[] size;
			delete[] rank;
			delete[] toParent;
		}

		// Public Methods
		void clear()
		{
			#pragma omp parallel for
			for (int i = 0; i < N; ++i)
			{
				parent[i] = i;
				size[i] = 1;
				rank[i] = 1;
				toParent[i] = 1;	// the weight to oneself is 1
			}
		}

		int find(const int node)
		{
			if (parent[node] == node)
				return node;
			else
			{
				// find parent of parent
				int p = find(parent[node]);

				// calculate the weight to root
				// Example:
				//		1 * 1 = 1
				//		1 * -1  = -1 or -1 * 1 = -1
				//		-1 * -1 = 1
				const int me_to_parent_weight = toParent[node];
				const int parent_to_root_weight = toParent[parent[node]];
				const int me_to_root_weight = me_to_parent_weight * parent_to_root_weight;

				// path compression
				toParent[node] = me_to_root_weight;
				parent[node] = p;
				return p;
			}
		}

		bool union_set(const int node1, const int node2)
		{
			int p1 = find(node1);
			int p2 = find(node2);

			// return true if set are merged
			// return false if set are unchanged
			if (p1 == p2)
				return false;
			else
			{
				union_by_size(p1, p2);
				return true;
			}
		}

		bool union_set(const int node1, const int node2, const int w)
		{
			// This is used in finding the MST
			// union the edge between node1 and node2 with weight w
			int p1 = find(node1);
			int p2 = find(node2);

			// return true if set are merged
			// return false if set are unchanged
			if (p1 == p2)
				return false;
			else
			{
				int new_parent = union_by_size(p1, p2);
				/** Uncomment if union by rank
				int new_parent = union_by_rank(p1, p1);
				*/
				
				int new_child = (new_parent == p1)?p2:p1;
				toParent[new_child] = w * toParent[node2] * toParent[node1];

				return true;
			}
		}

	private:
		int union_by_size(int p1, int p2)
		{
			if (size[p1] < size[p2])
				std::swap(p1, p2);

			parent[p2] = p1;
			size[p1] += size[p2];
			
			// return new parent after union
			return p1;
		}

		int union_by_rank(int p1, int p2)
		{
			if (rank[p1] < rank[p2])
				std::swap(p1, p2);
			
			parent[p2] = p1;
			++rank[p1];

			// return new parent after union
			return p1;
		}
};


#endif
