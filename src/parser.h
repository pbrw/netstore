#pragma once 

#include <string>
#include "boost/program_options.hpp"

using namespace std;
namespace po = boost::program_options;

// Parsing command line arguments
// using boost program options
void parse_args_server(int argc, char** argv);

void parse_args_client(int argc, char** argv);


// Parsing client input
class ParsingResult {
public:
	// command + argument (can be empty)
	std::string cmd;
	std::string arg;

	ParsingResult(std::string command, std::string arg);
};

ParsingResult parse_line(std::string line);

// test if command and argument match specific commands

bool match_discover(std::string& cmd, std::string& arg);

bool match_search(std::string& cmd, std::string& arg);

bool match_fetch(std::string& cmd, std::string& arg);

bool match_upload(std::string& cmd, std::string& arg);

bool match_remove(std::string& cmd, std::string& arg);

bool match_exit(std::string& cmd, std::string& arg);