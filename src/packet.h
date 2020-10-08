#pragma once

#include <string>
#include <vector>
#include <netdb.h>
#include <string.h>

// represents packet in format defined in the task statement
// SIMPL_CMD or CMPLX_CMD
class Packet {
public:
	std::string cmd;
	uint64_t cmd_seq;
	uint64_t param;
	std::string data;
	// is complex or simple
	bool cmplx;
	// sender address
	sockaddr_in addr;

	// construct SIMPL_CMD
	Packet(std::string cmd, uint64_t cmd_seq, std::string data);

	// construct CMPLX_CMD
	Packet(std::string cmd, uint64_t cmd_seq, uint64_t param, std::string data);

	// parse received packet
	Packet(sockaddr_in address, std::string data);

	// empty packet
	Packet();

	// get sender ip
	std::string get_ip();

	// get sender port
	uint16_t get_port();

	// represent as a string of bytes
	std::string convert_to_bytes();

	// returns number of bytes to allocate in the data section
	int get_free_data_space();
};