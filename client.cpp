#include <algorithm>
#include <sys/socket.h>
#include <netdb.h>

#include "client.h"
#include "display.h"
#include "transfer.h"

using namespace std;

Client& Client::get() {
	static Client instance;
	return instance;
}

void Client::init_sock() {
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) 
		throw socket_exception("Socket init error");

	// activate broadcast
  int optval = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void*)&optval, sizeof optval) < 0)
    throw socket_exception("setsockopt error");

  // activate multicast and set TTL
  optval = 4; 
  if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&optval, sizeof optval) < 0)
    throw socket_exception("setsockopt error");

	SOCK = sock;
}

Client::Client() {
	cmd_seq = 0;
}

void Client::sort_servers_by_free_space() {
	sort(servers.begin(), servers.end(), [](ServerData x, ServerData y) {
		return x.free_space > y.free_space;
	});
}

void Client::clear_shared_files() {
	for (auto& it : servers)
		it.shared_files.clear();
}

void Client::udp_send(Packet p) {
	// prepare server address
	sockaddr_in srvr_addr;
	string mcast_addr = Client::get().MCAST_ADDR;
	int cmd_port = Client::get().CMD_PORT;
	int sock = Client::get().SOCK;
	srvr_addr = transfer::get_address(mcast_addr, cmd_port);

	transfer::udp_send(p, srvr_addr, sock);
}

void Client::udp_send(Packet p, string ip_unicast) {
	// prepare server address
	sockaddr_in srvr_addr;
	int cmd_port = Client::get().CMD_PORT;
	int sock = Client::get().SOCK;
	srvr_addr = transfer::get_address(ip_unicast, cmd_port);

	transfer::udp_send(p, srvr_addr, sock);
}

Packet Client::udp_receive() {
	return transfer::udp_receive(SOCK, false);
}

void Client::tcp_receive(sockaddr_in sender_address, int fd) {
	int sock = transfer::tcp_connect(sender_address);
	transfer::tcp_receive(sock, fd);
}

void Client::tcp_send(sockaddr_in receiver_address, int fd) {
	int sock = transfer::tcp_connect(receiver_address);
	transfer::tcp_send(sock, fd);
}

int Client::find_server_index(ServerData server) {
	for (int i = 0; i < (int)servers.size(); i++) {
		if (servers[i] == server)
			return i;
	}

	return -1;
}