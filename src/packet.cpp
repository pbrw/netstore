#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <set>
#include <endian.h>

#include "packet.h"
#include "validation.h"
#include "display.h"
#include "command.h"	

constexpr int CMD_SEQ_SIZE = 8;
constexpr int PARAM_SIZE = 8;
constexpr int CMD_SIZE = 10;
constexpr int DATA_SIZE =	65400;

using namespace std;

set<string> cmplx_cmds = {"GOOD_DAY", "CONNECT_ME", "ADD", "CAN_ADD"};

Packet::Packet() {
	cmd = "EMPTY";
	cmd_seq = 0;
	param = 0;
	data = "";
	cmplx = false;
}

Packet::Packet(string cmd, uint64_t cmd_seq, string data) :
	cmd(cmd), cmd_seq(cmd_seq), param(0), data(data), cmplx(false) {
}

Packet::Packet(string cmd, uint64_t cmd_seq, uint64_t param, string data) :
	cmd(cmd), cmd_seq(cmd_seq), param(param), data(data), cmplx(true) {
}

Packet::Packet(struct sockaddr_in address, std::string bytes) {
	addr = address;
	int port = get_port();
	string ip = get_ip();

	try {
		validate_simpl_cmd_size(bytes);
	} catch (pckg_error_exception& e) {
		e.ip = ip;
		e.port = port;
		throw e;
	}

	int i = 0;

	// unpack command name field
	for (; i < CMD_SIZE; i++) {
		if (bytes[i])
			cmd += bytes[i];
	}

	// unpack command seq field
	uint8_t* byte = (uint8_t*)&cmd_seq;
	for (int j = 0; j < CMD_SEQ_SIZE; j++, i++) {
		*byte = bytes[i];
		byte++;
	}
	cmd_seq = be64toh(cmd_seq);

	// unpack param if necessary field
	if (cmplx_cmds.find(cmd) != cmplx_cmds.end()) {
		try {
			validate_cmplx_cmd_size(bytes);
		} catch (pckg_error_exception& e) {
			e.ip = ip;
			e.port = port;
			throw e;
		}
		cmplx = true;
		byte = (uint8_t*)&param;
		for (int j = 0; j < PARAM_SIZE; i++, j++) {
			*byte = bytes[i];
			byte++;
		}
		param = be64toh(param);
	} else {
		param = 0;
		cmplx = false;
	}

	// unpack data field
	data = "";
	for (int j = 0; i < (int)bytes.size(); i++, j++) {
		data += char(bytes[i]);
	}
}


std::string Packet::convert_to_bytes() {
	uint8_t* byte;
	string res = cmd;

	// put command name
	for (int i = res.size(); i < CMD_SIZE; i++) {
		res += char(0);
	}

	// put command seq
	uint64_t cp_cmd_seq = htobe64(cmd_seq);
	byte = (uint8_t*)&cp_cmd_seq;
	for (int i = 0; i < CMD_SEQ_SIZE; i++, byte++) {
		res += *byte;
	}

	// put param if necessary
	if (cmplx) {
		uint64_t cp_param = htobe64(param);
		byte = (uint8_t*)&cp_param;
		for (int i = 0; i < PARAM_SIZE; i++, byte++) {
			res += *byte;
		}
	}

	// put data
	res += data;

	return res;
}

int Packet::get_free_data_space() {
	return DATA_SIZE - (int)data.size();
}

string Packet::get_ip() {
	string res = "";
	char *ip = inet_ntoa(addr.sin_addr);
	while (*ip != char(0)) {
		res += *ip;
		ip++;
	}
	return res;
}

uint16_t Packet::get_port() {
	return ntohs(addr.sin_port);
}



