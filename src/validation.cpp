#include "validation.h"
#include "files.h"
#include "client.h"
#include "display.h"

#include <dirent.h>
#include <errno.h>
#include <sstream>

using namespace std;

constexpr int MAX_FILE_NAME_LEN = 255;

void validate_simpl_cmd_size(std::string& bytes) {
	if ((int)bytes.size() < SIMPL_CMD_SIZE)
		throw pckg_error_exception("Package too small");
}

void validate_cmplx_cmd_size(std::string& bytes) {
	if ((int)bytes.size() < CMPLX_CMD_SIZE)
		throw pckg_error_exception("Package too small for complex package");
}

void validate_hello_packet(Packet p) {
	if (!p.data.empty())
		throw pckg_error_exception(p, "Data section is not empty in HELLO command");
}

void validate_get_packet(Packet p) {
	if (p.data.empty())
		throw pckg_error_exception(p, "Empty file name in GET command");
	if (!in_storage(p.data))
		throw pckg_error_exception(p, "File " + p.data + " does not exist");	
}

void validate_del_packet(Packet p) {
	if (p.data.empty())
		throw pckg_error_exception(p, "Empty file name in DEL command");
}

void validate_add_packet(Packet p) {
	if (p.data.empty())
		throw pckg_error_exception(p, "Empty file name in ADD command");
}

void validate_good_day_packet(Packet p) {
	validate_cmd_seq(p);
	if (p.cmd != "GOOD_DAY")
		throw pckg_error_exception(p, "Unknown command");
}

void validate_my_list_packet(Packet p) {
	validate_cmd_seq(p);
	if (p.cmd != "MY_LIST")
		throw pckg_error_exception(p, "Unknown command");
	if (p.data.empty())
		throw pckg_error_exception(p, "Empty data section in MY_LIST packet");
	if (p.data.back() == '\n')
		throw pckg_error_exception(p, "New line at the end of files list");
}

void validate_connect_me_packet(Packet p, std::string file_name) {
	validate_cmd_seq(p);
	if (p.cmd != "CONNECT_ME")
		throw pckg_error_exception(p, "Unknown command");
	if (p.data != file_name)
		throw pckg_error_exception(p, "File names does not match");
}

void validate_cmd_seq(Packet p) {
	if (p.cmd_seq != Client::get().cmd_seq)
		throw pckg_error_exception(p, "Invalid cmd_seq");
}

void validate_can_add_packet(Packet p) {
	validate_cmd_seq(p);
	if (p.cmd != "CAN_ADD")
		throw pckg_error_exception(p, "Unknown command");
	if (!p.data.empty())
		throw pckg_error_exception(p, "Data section is not empty in CAN_ADD packet");
}

void validate_no_way_packet(Packet p, std::string file_name) {
	validate_cmd_seq(p);
	if (p.cmd != "NO_WAY")
		throw pckg_error_exception(p, "Unknown command");
	if (p.data != file_name)
		throw pckg_error_exception(p, "File names does not match");
}

void validate_folder(std::string folder_path) {
	DIR* dir = opendir(folder_path.c_str());
	if (dir) {
		closedir(dir);
	} else if (ENOENT == errno) {
		throw options_exception("Directory does not exist");
	} else {
		throw options_exception("Could not open directory");
	}
}

bool is_digit(char c) {
	return c >= '0' && c <= '9';
}

void validate_port(std::string port) {
	options_exception e("Invalid port number");

	if (port[0] == '-') {
		throw e;
	}

	for (char c :  port) {
		if (!is_digit(c)) {
			throw e;
		}
	}

	int port_num = stoi(port);

	if (port_num < 1 || port_num > 65535) {
		throw e;
	}
}

void validate_ip(std::string ip) {
	options_exception e("Invalid IP address");
	string num = "";
	int cnt = 0;
	ip += ".";

	for (char c : ip) {
		if (c == '.') {
			cnt++;
			int tmp = stoi(num);
			if (tmp < 0 || tmp > 255)
				throw e;
			num = "";
			continue;
		}
		
		if (!is_digit(c)) {
			throw e;
		}

		num += c;
	}

	if (cnt != 4)
		throw e;
}

void validate_timeout(string timeout) {
	options_exception e("Invalid timeout");

	if (timeout[0] == '-') {
		throw e;
	}

	for (char c :  timeout) {
		if (!is_digit(c)) {
			throw e;
		}
	}

	int to_num = stoi(timeout);

	if (to_num < 1 || to_num > 300) {
		throw e;
	}
}

void validate_max_space(string max_space) {
	options_exception e("Invalid max space");

	if (max_space[0] == '-') {
		throw e;
	}

	for (char c : max_space) {
		if (!is_digit(c)) {
			throw e;
		}
	}
}

bool valid_file_name(std::string file_name) {
	if (file_name == "") return false;
	if (file_name.size() > MAX_FILE_NAME_LEN) return false;
	if (file_name.find(char(0)) != string::npos) return false;

	return true;
}
