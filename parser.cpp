#include <ctype.h>
#include <iostream>

#include "parser.h"
#include "server.h"
#include "client.h"
#include "validation.h"

using namespace std;

void parse_args_server(int argc, char** argv) {
	po::options_description desc("Options"); 
	desc.add_options() 
		("help,help", "produce help message")
    ("p,p", po::value<string>()->required(), "UDP port number")
		("g,g", po::value<string>()->required(), "Multicast address")
		("b,b", po::value<string>(), "Maximum disk space")
		("f,f", po::value<string>()->required(), "Shared folder path")
		("t,t", po::value<string>(), "Timeout in seconds");
	
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	

	if (vm.count("help")) {
		cout << desc << "\n";
	}

	if (vm.count("p")) {
		string port = vm["p"].as<string>();
		validate_port(port);
		Server::get().CMD_PORT = stoi(port); 
	}

	if (vm.count("g")) {
		string ip = vm["g"].as<string>();
		validate_ip(ip);
		Server::get().MCAST_ADDR = ip; 
	}

	if (vm.count("b")) {
		string max_space = vm["b"].as<string>();
		validate_max_space(max_space);
		Server::get().MAX_SPACE = stoll(max_space);
	} else {
		Server::get().MAX_SPACE = 52428800;
	}

	if (vm.count("f")) {
		string shrd_fldr = vm["f"].as<string>();
		if (shrd_fldr.back() != '/') { 
			shrd_fldr.push_back('/');
		}
		validate_folder(shrd_fldr);
		Server::get().SHRD_FLDR = shrd_fldr;
	}

	if (vm.count("t")) {
		string timeout = vm["t"].as<string>();
		validate_timeout(timeout);
 		Server::get().TIMEOUT = stoi(timeout);
	} else {
		Server::get().TIMEOUT = 5;
	}
}

void parse_args_client(int argc, char** argv) {
	po::options_description desc("Options"); 
	desc.add_options() 
		("help", "produce help message")
    ("p,p", po::value<string>()->required(), "UDP port number")
		("g,g", po::value<string>()->required(), "Multicast address")
		("o,o", po::value<string>()->required(), "Out folder path")
		("t,t", po::value<string>(), "Timeout in seconds");
	
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cout << desc << "\n";
	}

	if (vm.count("p")) {
		string port = vm["p"].as<string>();
		validate_port(port);
		Client::get().CMD_PORT = stoi(port);
	} 

	if (vm.count("g")) {
		string ip = vm["g"].as<string>();
		validate_ip(ip);
		Client::get().MCAST_ADDR = ip; 
	} 

	if (vm.count("o")) {
		string out_fldr = vm["o"].as<string>();
		if (out_fldr.back() != '/') { 
			out_fldr.push_back('/');
		}
		validate_folder(out_fldr);
		Client::get().OUT_FLDR = out_fldr;
	} 

	if (vm.count("t")) {
		string timeout = vm["t"].as<string>();
		validate_timeout(timeout);
 		Client::get().TIMEOUT = stoi(timeout);
	} else {
		Client::get().TIMEOUT = 5;
	}
}

ParsingResult::ParsingResult(std::string command, std::string arg) :
	cmd(command), arg(arg) {
}

ParsingResult parse_line(string line) {
	string command = "";
	int i = 0;
	for (; i < (int)line.size(); i++) {
		if (line[i] == char(32))
			break;
		if (line[i] == char(10))
			break;

		command += line[i];
	}

	string arg = "";
	for (; i < (int)line.size(); i++) {
		if (line[i] == char(10))
			break;
		arg += line[i];
	}

	for (char& c : command)
		c = tolower(c);

	if (match_discover(command, arg)) {}
	else if (match_search(command, arg)) {}
	else if (match_fetch(command, arg)) {}
	else if (match_upload(command, arg)) {}
	else if (match_remove(command, arg)) {}
	else if (match_exit(command, arg)) {}
	else {
		return ParsingResult("unknown", "");
	}

	return ParsingResult(command, arg);
}


bool match_discover(std::string& cmd, std::string& arg) {
	return (cmd == "discover" && arg == "");
}

bool match_search(std::string& cmd, std::string& arg) {
	if (cmd != "search")
		return false;
	
	if (arg == "") {}
	else if (arg == " ") {
		arg = "";
	} else {
		arg = arg.substr(1);
	}

	return true;
}

bool match_fetch(std::string& cmd, std::string& arg) {
	if (cmd != "fetch" || arg.size() < 2)
		return false;
	
	arg = arg.substr(1);

	return true;
}

bool match_upload(std::string& cmd, std::string& arg) {
	if (cmd != "upload" || arg.size() < 2)
		return false;
	
	arg = arg.substr(1);

	return true;
}

bool match_remove(std::string& cmd, std::string& arg) {
	if (cmd != "remove" || arg.size() < 2)
		return false;
	
	arg = arg.substr(1);

	return true;
}

bool match_exit(std::string& cmd, std::string& arg) {
	return (cmd == "exit" && arg == "");
}


