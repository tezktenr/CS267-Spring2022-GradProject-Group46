/********
 * INCLUDE HEADER
 ***/
#include <iostream>
#include <fstream>

/********
 * PRE-PROCESSOR
 ***/
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
		std::cout << "Command Error: You must specify the graph file name via '-f'" std::endl;
		return 1;
	}
	#ifdef DEBUG_MAIN
	std::cout << "graph file name: " << graph_filepath << std::endl;
	#endif
	std::ifstream graph_txt(graph_filepath);
	


	return 0;
}
