#include <iostream>  
#include <string>
#include <cstdio>

#include "boost/program_options.hpp" 

#include "parser.h"
#include "server.h"
#include "validation.h"
#include "display.h"
#include "packet.h"
#include "command.h"
#include "transfer.h"

using namespace std;
namespace po = boost::program_options;


int main(int argc, char** argv) {
	// try to parse commmand line arguments 
	try {
		parse_args_server(argc, argv);
	} catch (po::error& e) {
		cerr << e.what() << endl;
		return 1;
	} catch (options_exception& e) {
		e.info();
		return 1;
	}

	// try to init socket and index files in shared folder
	try {
		Server::get().init_sock();
	} catch (socket_exception& e) {
		e.info();
		return 1;
	}

	Server::get().index_files();
	
	while (1) {
		Packet p;
		try {
			// receive packet
			p = Server::get().udp_receive();
		
			// execute proper command
			if (p.cmd == "HELLO") {
				discover_resp(p);
			} else if (p.cmd == "LIST") {
				search_resp(p);
			} else if (p.cmd == "DEL") {
				remove_resp(p);	
			} else if (p.cmd == "GET") {
				fetch_resp(p);
			} else if (p.cmd == "ADD") {
				upload_resp(p);
			} else {
				pckg_error_exception e("Unknown command");
				e.ip = p.get_ip();
				e.port = p.get_port();
				throw e;
			}
		} catch (pckg_error_exception& e) {
			// diplay package error
			e.info(Server::get().output_mutex);
		}

	}

	return 0;
}