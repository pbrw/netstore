#pragma once 

#include <string>
#include <exception>
#include <mutex>

#include "packet.h"

constexpr int SIMPL_CMD_SIZE = 18;
constexpr int CMPLX_CMD_SIZE = 26;

void validate_simpl_cmd_size(std::string& bytes);

void validate_cmplx_cmd_size(std::string& bytes);

void validate_hello_packet(Packet p);

void validate_get_packet(Packet p);

void validate_del_packet(Packet p);

void validate_add_packet(Packet p);

void validate_good_day_packet(Packet p);

void validate_my_list_packet(Packet p);

void validate_connect_me_packet(Packet p, std::string file_name);

void validate_cmd_seq(Packet p);

void validate_can_add_packet(Packet p);

void validate_no_way_packet(Packet p, std::string file_name);

void validate_folder(std::string folder_path);

void validate_port(std::string port);

void validate_ip(std::string ip);

void validate_timeout(std::string timeout);

void validate_max_space(std::string max_space);

bool valid_file_name(std::string file_name);

