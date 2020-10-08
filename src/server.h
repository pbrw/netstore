#pragma once

#include <string>
#include <set>
#include <mutex>

#include "packet.h"

#include <sys/types.h>
// #include <netdb.h>

// Singleton which consists all the data and functions necessary to 
// run server application
class Server {
public:
	std::string 		MCAST_ADDR;
	int							CMD_PORT;
	long long				MAX_SPACE;
	std::string 		SHRD_FLDR;
	int			    		TIMEOUT;
	// server's UDP socket
	int 						SOCK;

	// set of names of files stored
	std::set<std::string> shared_files;
	// mutex guadring stdout and stderr
	std::mutex output_mutex;

	static Server& get();

	// initialize socket
	void init_sock();

	// index file names in storage
	void index_files();

	// receive UDP packet
	Packet udp_receive();

	// send a packet to receiver with UDP
	void udp_send(Packet p, sockaddr_in receiver_address);

	// receive a string of bytes from sock and save it to file descriptor
	void tcp_receive(int sock, int fd);

private:
	Server();
};