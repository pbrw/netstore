#pragma once

#include <string>
#include <mutex>

#include "packet.h"
#include "serverData.h"

// Tools for displaying messages to console

// debug mode switch
constexpr int DEBUG = 0;

// exception thrown after fail in data transfer
class file_transfer_exception: public std::exception {
	public:
	uint16_t port;
	std::string ip;
	std::string note;
	std::string file_name;
	// download = 0, upload = 1
	bool direction;

	file_transfer_exception(std::string note);

	// display error description
	void info(std::mutex& m);
};

// exception thrown after fail in working with sockets
class socket_exception: std::exception {
public:
	std::string note;
	
	socket_exception(std::string note);

	// display error description
	void info();
};

// exception thrown after fail in working with sockets
class filesystem_exception: std::exception {
public:
	std::string note;
	
	filesystem_exception(std::string note);

	// display error description
	// only if debug mode is enabled
	void info();
};

// exception thrown after receving incorrect package(packet) format
class pckg_error_exception: public std::exception {
public:
	uint16_t port;
	std::string ip;
	std::string note;

	pckg_error_exception(std::string note);

	pckg_error_exception(Packet p, std::string note);
	
	// display error description
	void info(std::mutex& m);
};

// exception thrown after typing invalid options
class options_exception: std::exception {
public:
	std::string note;
	options_exception(std::string note);

	// display error description
	void info();
};

// basic error message used in file transfer management
// displayed only if debug mode is enabled
void error(std::string text);

// entry with found server
void found_info(ServerData server);

// entry with found file
void file_entry(std::string file_name, std::string ip);

void file_downloaded(std::string file_name, std::string ip, uint64_t port);

void file_uploaded(std::string file_name, std::string ip, uint64_t port);

// display all found files in search
void display_files();

void file_too_big(std::string file_name);

void file_not_exist(std::string file_name);

// print message to console with mutex (thread-safe)
void sync_output(std::string text, std::mutex& m, std::ostream& os);
