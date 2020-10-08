#pragma once

#include <string>

#include "packet.h"

// Client commands top-level logic

void discover();

void search(std::string substr);

void remove_cmd(std::string file_name);

void fetch(std::string file_name);

void upload(std::string file_path);


// Server responses top-level logic
// req - packet that requested command

void discover_resp(Packet req);

void search_resp(Packet req);

void remove_resp(Packet req);

void fetch_resp(Packet req);

void upload_resp(Packet req);

