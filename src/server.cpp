#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>

#include "server.h"
#include "display.h"
#include "transfer.h"
#include "files.h"

using namespace std;

Server& Server::get() {
	static Server instance;

	return instance;
}

void Server::index_files() {
	vector<string> file_names = get_file_names(SHRD_FLDR);
	for (auto file : file_names) {
		shared_files.insert(file);
		string file_path = SHRD_FLDR + file;

		// update max space
		uint64_t file_size = get_file_size(file_path);
		MAX_SPACE -= file_size;
	}
}

void Server::init_sock() {
	sockaddr_in server_address;
	ip_mreq ip_mreq;

	// create socket
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) 
		throw socket_exception("Socket init error");
	
  ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (inet_aton(MCAST_ADDR.c_str(), &ip_mreq.imr_multiaddr) == 0)
    throw socket_exception("inet_aton error");
  if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ip_mreq, sizeof(ip_mreq)) < 0)
    throw socket_exception("setsockopt error");

	server_address.sin_family = AF_INET; 
	server_address.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_address.sin_port = htons(CMD_PORT); 

	// bind the socket to a concrete address
	if (bind(sock, (struct sockaddr *) &server_address,
			(socklen_t) sizeof(server_address)) < 0)
		throw socket_exception("bind error");

	SOCK = sock;
}

Server::Server() {}

Packet Server::udp_receive() {
	return transfer::udp_receive(SOCK, true);
}

void Server::udp_send(Packet p, struct sockaddr_in receiver_address) {
	transfer::udp_send(p, receiver_address, SOCK);
}

void Server::tcp_receive(int sock, int fd) {
	transfer::tcp_receive(sock, fd);
}