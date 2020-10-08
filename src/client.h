#pragma once

#include <string>
#include <iostream>
#include <mutex>
#include <sys/types.h>

#include "serverData.h"

// Singleton which consists all the data and functions necessary to 
// run client application
class Client {
public:
	std::string 		MCAST_ADDR;
	int							CMD_PORT;
	std::string 		OUT_FLDR;
	int			    		TIMEOUT;
	int 						SOCK;
	
	static Client& get();

	// last CMD_SEQ number sent
	uint64_t cmd_seq;
	// information about known servers
	std::vector<ServerData> servers;
	// mutex guarding stdout and stderr
	std::mutex output_mutex;

	// initialize UDP socket
	void init_sock();

	void sort_servers_by_free_space();

	// for every server clear the information about shared files
	void clear_shared_files();

	// send packet with multicast
	void udp_send(Packet p);

	// send packet with unicast
	void udp_send(Packet p, std::string ip_unicast);

	Packet udp_receive();

	// save a string of bytes from sender with TCP in fd
	void tcp_receive(sockaddr_in sender_address, int fd);

	// send file content to receiver with TCP
	void tcp_send(sockaddr_in receiver_address, int fd);

	// returns index of the server in servers vector
	// if there is no such server returns -1
	int find_server_index(ServerData server);

private:
	Client();
};