/********
 * INCLUDE HEADER
 ***/
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <chrono>
#include "graph.hpp"

/********
 * PRE-PROCESSOR
 ***/
#define IMPL_NAME "Naive"
//#define DEBUG_MAIN


/********
 * GLOBAL MEM
 ***/



/********
* HELPER FUNCTIONS
***/
// Command Line Option Processing
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
        std::cout << "Options:" << std::endl;
        std::cout << "-h: see this help" << std::endl;
        std::cout << "-f <filename>: refer to the graph file name" << std::endl;
        return 0;
    }

	// Parse graph file name
	char* graph_filepath = find_string_option(argc, argv, "-f", nullptr);
	if (graph_filepath == nullptr)
	{
		std::cout << "Command Error: You must specify the graph file name via '-f'" << std::endl;
		return 1;
	}	
	Graph_t graph(graph_filepath);
	#ifdef DEBUG_MAIN
	std::cout << "| " << graph_filepath << std::endl;
	std::string test_line;
	std::ifstream test_txt(graph_filepath);
	std::cout << "+--------------" << std::endl;
	while (std::getline(test_txt, test_line)) std::cout << "| " << test_line << std::endl;
	std::cout << std::endl;
	test_txt.close();
	#endif


	// Main Algorithms	
	auto start_time = std::chrono::steady_clock::now();




	auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end_time - start_time;
    double seconds = diff.count();

    // Finalize
    std::cout << "Runtime (" << IMPL_NAME << ") = " << seconds << " seconds" << std::endl;

	return 0;
}
