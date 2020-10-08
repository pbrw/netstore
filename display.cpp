#include <iostream>
#include <sstream>

#include "display.h"
#include "client.h"


using namespace std;

pckg_error_exception::pckg_error_exception(string note): note(note)  {

}

pckg_error_exception::pckg_error_exception(Packet p, string note): note(note)  {
	port = p.get_port();
	ip = p.get_ip();
}

void pckg_error_exception::info(mutex& m) {
	stringstream ss;
	ss << "[PCKG ERROR] Skipping invalid package from ";
	ss << ip + ":" + to_string(port) + ". " + note;
	ss << endl;
	sync_output(ss.str(), m, cerr);
}

options_exception::options_exception(string note): note(note) {}

void options_exception::info() {
	stringstream ss;
	ss << "[ERROR] " << note << endl;
	cerr << ss.str();
}

void error(string text) {
	if (!DEBUG)
		return;
	cerr << "[ERROR] " << text << endl;
}

void found_info(ServerData server) {
	stringstream ss;
	ss << "Found " << server.ip_unicast << " (" << server.ip_multicast 
		<< ") with free space " 
		<< server.free_space << endl;
	sync_output(ss.str(), Client::get().output_mutex, cout);
}

void file_entry(string file_name, string ip) {
	stringstream ss;
	ss << file_name << " (" << ip << ")" << endl;
	sync_output(ss.str(), Client::get().output_mutex, cout);
}

void file_downloaded(string file_name, string ip, uint64_t port) {
	stringstream ss;
	ss << "File " << file_name << " downloaded (" << ip << ":" << port << ")" << endl;
	sync_output(ss.str(), Client::get().output_mutex, cout);
}

void file_uploaded(string file_name, string ip, uint64_t port) {
	stringstream ss;
	ss << "File " << file_name << " uploaded (" << ip << ":" << port << ")" << endl;
	sync_output(ss.str(), Client::get().output_mutex, cout);
}

void display_files() {
	for (auto& it : Client::get().servers) {
		for (auto& f : it.shared_files) {
			file_entry(f, it.ip_unicast);
		}
	}
}

file_transfer_exception::file_transfer_exception(string note) : 
	note(note) {
}

void file_transfer_exception::info(mutex& m) {
	stringstream ss;
	ss << "File " << file_name << " ";
	ss << (direction ? "uploading" : "downloading");
	ss << " failed ";
	ss << "(" + ip + ":" + (port == 0 ? "" : to_string(port)) + ") " + note << endl;
	sync_output(ss.str(), m, cout);
}

void file_too_big(string file_name) {
	stringstream ss;
	ss << "File " << file_name << " too big" << endl;
	sync_output(ss.str(), Client::get().output_mutex, cout);
}

void file_not_exist(string file_name) {
	stringstream ss;
	ss << "File " << file_name << " does not exist" << endl;
	sync_output(ss.str(), Client::get().output_mutex, cout);
}

socket_exception::socket_exception(string note) : 
	note(note) {
}

void socket_exception::info() {
	cerr << "[ERROR] " << note << endl;
}

filesystem_exception::filesystem_exception(string note) : 
	note(note) {
}

void filesystem_exception::info() {
	if (!DEBUG) return;  
	cerr << "[ERROR] " << note << endl;
}

void sync_output(string text, mutex& m, ostream& os) {
	m.lock();
	os << text;
	m.unlock();
}