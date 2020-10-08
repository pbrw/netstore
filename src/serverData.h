#pragma once

#include <string>
#include <vector>
#include <arpa/inet.h>

#include "packet.h"

// stores necessary information about server (client side)
class ServerData {
	public:
		// list of names of files on this server
		std::vector<std::string> shared_files;		
		std::string ip_unicast;
		std::string ip_multicast;
		uint64_t port;
		uint64_t free_space;

		// save data based on received packet
		ServerData(Packet p);

		void add_file(std::string file_name);

		bool has_file(std::string file_name);

		// servers are compared with unicast and port
		bool operator==(ServerData const &hd);
};