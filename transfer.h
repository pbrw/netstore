#pragma once

#include <string>

#include "packet.h"

// Functions responsible for transfering packets and data with UDP/TCP

namespace transfer {

// returns sockaddr_in structure with ip and port
sockaddr_in get_address(std::string ip, uint16_t port);

// send UDP packet to receiver
void udp_send(Packet packet, struct sockaddr_in receiver_address, int sock);

// receive packet from from in a blocking or non-blocking way
Packet udp_receive(int sock, bool blocking);

// listen for TCP connections on socket sock
void tcp_listen(int sock);

// accept TCP connection on socket SOCK and return msg_sock
int tcp_accept(int sock);

// connect TCP to server_address
int tcp_connect(struct sockaddr_in server_address);

// send data from file descriptor to TCP socket
void tcp_send(int sock, int fd);

// save data from TCP socket to file descriptor
void tcp_receive(int sock, int fd);

// get random blocking or not free TCP socket
int get_free_tcp_socket(bool blocking);

// get random blocking or not free TCP port
int get_free_tcp_port(int sock);

}