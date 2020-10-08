#include "transfer.h"
#include "server.h"
#include "display.h"
#include "client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iomanip>
#include <ctime>
#include <chrono>

constexpr int BUFFER_SIZE = 65507;
constexpr int QUEUE_LENGTH = 5;

using namespace std;
using namespace chrono;

namespace transfer {

struct sockaddr_in get_address(string ip, uint16_t port) {
	struct addrinfo addr_hints;
  struct addrinfo *addr_result;
	struct sockaddr_in res;

	(void) memset(&addr_hints, 0, sizeof(struct addrinfo));
  addr_hints.ai_family = AF_INET; 
  addr_hints.ai_socktype = SOCK_DGRAM;
  addr_hints.ai_protocol = IPPROTO_UDP;
  if (getaddrinfo(ip.c_str(), NULL, &addr_hints, &addr_result) != 0) {
    error("getaddrinfo");
  }

  res.sin_family = AF_INET; 
  res.sin_addr.s_addr =
      ((struct sockaddr_in*) (addr_result->ai_addr))->sin_addr.s_addr; // address IP
  res.sin_port = htons(port); // port from the command line

  freeaddrinfo(addr_result);

	return res;
}

void udp_send(Packet packet, struct sockaddr_in receiver_address, int sock) {
	string data = packet.convert_to_bytes();
	
	if (data.size() > BUFFER_SIZE) {
		error("Message too long");
	}

	socklen_t rcva_len = (socklen_t) sizeof(receiver_address);
	ssize_t snd_len = sendto(sock, data.c_str(), data.size(), 0,
		(struct sockaddr *) &receiver_address, rcva_len);

	if (snd_len != (ssize_t) data.size())
		error("Not fully sent");
}

Packet udp_receive(int sock, bool blocking) {
	struct sockaddr_in sender_address;
	char buffer[BUFFER_SIZE];
	
	int flags = blocking ? 0 : MSG_DONTWAIT;
	socklen_t rcva_len = (socklen_t) sizeof(sender_address);
	ssize_t len = recvfrom(sock, buffer, sizeof(buffer), flags,
			(struct sockaddr *) &sender_address, &rcva_len);
	
	if (len < 0) {
		if (blocking) 
			error("Error on datagram from sender socket");
		else 
			return Packet("EMPTY", 0, "");
	}
	
	string data(buffer, len);

	return Packet(sender_address, data);
}

int get_free_tcp_port(int sock) {
	// get random free port by plugging sin_port = 0
	struct sockaddr_in server_address;
	
	server_address.sin_family = AF_INET; 
  server_address.sin_addr.s_addr = htonl(INADDR_ANY); 
  server_address.sin_port = htons(0);

	socklen_t len = sizeof(server_address);

	if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    error("TCP bind");

	if (getsockname(sock, (struct sockaddr *)&server_address, &len) < 0)
		error("getsockname");
	
	return ntohs(server_address.sin_port);
}

int get_free_tcp_socket(bool blocking) {
	int flags = SOCK_STREAM;
	flags |= blocking ? 0 : SOCK_NONBLOCK;
	int sock = socket(PF_INET, flags, 0);
  if (sock < 0)
    error("TCP socket");

	return sock;
}

void tcp_listen(int sock) {
	if (listen(sock, QUEUE_LENGTH) < 0) {
    (void) close(sock);
		throw file_transfer_exception("TCP listen error");
	}
}

int tcp_accept(int sock) {
	int msg_sock = -1;

	// wait TIMEOUT seconds for TCP connection
	auto begin = high_resolution_clock::now();
	while (1) {
		auto now = high_resolution_clock::now();
		duration<double, std::milli> delta = now - begin;

		if (delta.count() > (double)Server::get().TIMEOUT * 1000)
			break;

		msg_sock = accept(sock, NULL, NULL);

		if (msg_sock > 0)
			break;
	}

	if (msg_sock < 0) {
		return msg_sock;
	}

	return msg_sock;
}

void tcp_send(int sock, int fd) {
	ssize_t snd_len;
	ssize_t total_send_len = 0, now_send_len = 0;
	ssize_t read_len, total_read_len = 0;

	char buffer[BUFFER_SIZE];

	// send packets in loop till full transfer
	do {
		read_len = read(fd, buffer, BUFFER_SIZE);
		now_send_len = 0;
		while (now_send_len < read_len) {
			snd_len = write(sock, buffer + now_send_len, read_len - now_send_len);
			if (snd_len < 0) {
				(void) close(sock);
				throw file_transfer_exception("Write to socket error");
			}
			now_send_len += snd_len;
		}
		total_send_len += now_send_len;
		total_read_len += read_len;
	} while (read_len > 0);

	if (total_send_len != total_read_len) {
		(void) close(sock);
		throw file_transfer_exception("TCP writing to client socket error");
	}

	if (close(sock) < 0)
		throw file_transfer_exception("TCP close error");
}

int tcp_connect(struct sockaddr_in server_address) {
	int sock = get_free_tcp_socket(true);

	socklen_t len = sizeof(server_address);
  if (connect(sock, (struct sockaddr *)&server_address, len) < 0) {
    (void) close(sock);
		throw file_transfer_exception("TCP connect error");
	}

	return sock;
}

void tcp_receive(int sock, int fd) {
  char buffer[BUFFER_SIZE];
  ssize_t rcv_len;
	ssize_t saved_len, total_saved_len = 0;

	// reading data while there is something coming
	do {
		rcv_len = read(sock, buffer, sizeof(buffer));
		total_saved_len = 0;
		while (total_saved_len < rcv_len) {
			saved_len = write(fd, buffer + total_saved_len, rcv_len - total_saved_len);
			if (saved_len < 0) {
				(void) close(sock);
				throw file_transfer_exception("Write to file descriptor error");
			}
			total_saved_len += saved_len;
		}
	} while (rcv_len > 0);

	if (rcv_len < 0) {
		(void) close(sock);
		throw file_transfer_exception("TCP read error");
	}
	
  (void) close(sock);
}

}