#include "serverData.h"

using namespace std;

ServerData::ServerData(Packet p) {
	ip_unicast = p.get_ip();
	port = p.get_port();
	free_space = p.param;
	ip_multicast = p.data;
}

void ServerData::add_file(std::string file_name) {
	shared_files.push_back(file_name);
}

bool ServerData::has_file(string file_name) {
	for (auto& f : shared_files) {
		if (f == file_name)
			return true;
	}
	
	return false;
}

bool ServerData::operator==(ServerData const &hd) {
	return ip_unicast == hd.ip_unicast && port == hd.port;
}