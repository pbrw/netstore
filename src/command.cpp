#include <iomanip>
#include <ctime>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <vector>

#include "command.h"
#include "display.h"
#include "serverData.h"
#include "server.h"
#include "client.h"
#include "transfer.h"
#include "files.h"
#include "validation.h"

using namespace std;
using namespace transfer;
using namespace chrono;


void discover() {
	// send multicast request
	Client::get().cmd_seq++;
	Packet req("HELLO", Client::get().cmd_seq, "");
	Client::get().udp_send(req);

	Packet res;
	vector<ServerData> new_servers;

	// receive incoming packets for TIMEOUT seconds
	auto begin = high_resolution_clock::now();
	while (1) {
		auto now = high_resolution_clock::now();
		duration<double, milli> delta = now - begin;
		if (delta.count() > (double)Client::get().TIMEOUT * 1000)
			break;
	
		res = Client::get().udp_receive();
		
		// update information about servers
		if (res.cmd != "EMPTY") {
			try {
				validate_good_day_packet(res);
				ServerData server(res);
				found_info(server);
				new_servers.push_back(server);
				int id = Client::get().find_server_index(server);
				if (id >= 0) {
					new_servers.back().shared_files = 
						Client::get().servers[id].shared_files;
				}
			} catch (pckg_error_exception& e) {
				e.info(Client::get().output_mutex);
			}
		}
	}

	Client::get().servers = new_servers;
}


void search(string substr) {
	// send multicast request
	Client::get().clear_shared_files();
	Client::get().cmd_seq++;
	Packet req("LIST", Client::get().cmd_seq, substr);
	Client::get().udp_send(req);

	vector<string> files;
	files.push_back("");

	// receive incoming packets for TIMEOUT seconds
	auto begin = high_resolution_clock::now();
	while (1) {
		auto now = high_resolution_clock::now();
		duration<double, milli> delta = now - begin;
		if (delta.count() > (double)Client::get().TIMEOUT * 1000)
			break;

		try {
			Packet res = Client::get().udp_receive();

			if (res.cmd == "EMPTY")
				continue;
		
			// unpack file names
			validate_my_list_packet(res);

			for (char c : res.data) {
				if ((int)c == 0)
					break;
				if (c == '\n')
					files.push_back("");
				else
					files.back() += c;
			}

			if (files.size() == 1 && files.back() == "")
				files.pop_back();
			
			// update servers information
			for (auto& it : Client::get().servers) {
				if (it.ip_unicast == res.get_ip() && it.port == res.get_port()) {
					for (auto f : files) {
						it.add_file(f);
					}
				}
			}

			files.clear();
			files.push_back("");
		} catch (pckg_error_exception& e) {
			e.info(Client::get().output_mutex);
		}
	}

	display_files();
}


void remove_cmd(string file_name) {
	Client::get().cmd_seq++;
	Packet req("DEL", Client::get().cmd_seq, file_name);
	Client::get().udp_send(req);
}


// download file from server with TCP
void download_file(Packet res) {
	// set up received port number
	res.addr.sin_port = htons(res.param);
	int fd = -1;
	try {
		// save content
		string file_path = Client::get().OUT_FLDR + res.data;
		
		fd = open_file(file_path);
		// get content
		Client::get().tcp_receive(res.addr, fd);

		close(fd);

		// save_file(file_path, data);
		file_downloaded(res.data, res.get_ip(), res.get_port());
	
	} catch (file_transfer_exception& e) {
		// diplay full error
		e.direction = false;
		e.file_name = res.data;
		e.ip = res.get_ip();
		e.port = res.get_port();
		e.info(Client::get().output_mutex);
		if (fd >= 0)
			close(fd);
	}
	if (fd >= 0)
		close(fd);
}


void fetch(string file_name) {
	// choose server to fetch file from
	string ip = "";
	for (auto& server : Client::get().servers) {
		if (server.has_file(file_name))
			ip = server.ip_unicast;
	}

	if (ip == "") {
		return;
	}	

	// send unicast request to download file
	Client::get().cmd_seq++;
	Packet req("GET", Client::get().cmd_seq, file_name);
	Client::get().udp_send(req, ip);

	Packet res;
	// receive permission from server with port number
	// receive incoming packets for TIMEOUT seconds
	auto begin = high_resolution_clock::now();
	while (1) {
		auto now = high_resolution_clock::now();
		duration<double, milli> delta = now - begin;
		if (delta.count() > (double)Client::get().TIMEOUT * 1000)
			break;
		
		try {
			res = Client::get().udp_receive();

			if (res.cmd != "EMPTY") {
				validate_connect_me_packet(res, file_name);
				thread th(download_file, res);
				th.detach();
				return;
			}
		} catch (pckg_error_exception& e) {
			e.info(Client::get().output_mutex);
		}
	}

	// display error if there was no answer
	file_transfer_exception e("Server not responding");
	e.direction = false;
	e.file_name = file_name;
	e.ip = ip;
	e.info(Client::get().output_mutex);
	
}


// receive permission and port number from server to upload a file
// returns recieved port number on success
// returns -1 on failure
uint64_t get_port_to_upload(Packet req, string ip) {
	// send request
	Client::get().cmd_seq++;	
	req.cmd_seq = Client::get().cmd_seq;
	Client::get().udp_send(req, ip);

	Packet res;
	// receive incoming packets for TIMEOUT seconds
	auto begin = high_resolution_clock::now();
	while (1) {
		auto now = high_resolution_clock::now();
		duration<double, milli> delta = now - begin;
		if (delta.count() > (double)Client::get().TIMEOUT * 1000)
			break;

		try {
			res = Client::get().udp_receive();

			if (res.cmd != "EMPTY") {
				// verify response
					if (res.cmd == "CAN_ADD") {
						validate_can_add_packet(res);
						return res.param;
					} else {
						validate_no_way_packet(res, req.data);
						return -1;
					}
			}
		} catch (pckg_error_exception& e) {
			e.info(Client::get().output_mutex);
		}
	}
	
	return -1;
}


// upload file to server with TCP
// args:
//	ip - server's unicast ip
// 	port - server's port
// 	name - name of the file
//  fd - file descriptor
void upload_file(string ip, uint64_t port, string name, int fd) {
	sockaddr_in addr = get_address(ip, port);
	try {
		Client::get().tcp_send(addr, fd);
		file_uploaded(name, ip, port);
	} catch (file_transfer_exception& e) {
		e.direction = true;
		e.file_name = name;
		e.ip = ip;
		e.port = port;
		e.info(Client::get().output_mutex);
	}

	close(fd);
}


void upload(string file_path) {
	// prepare file
	string file_name = get_file_name(file_path);
	
	if (!file_exists(file_path)) {
		file_not_exist(file_name);
		return;
	}

	long long file_size = get_file_size(file_path);

	Client::get().sort_servers_by_free_space();

	Packet req("ADD", 0, file_size, file_name);

	bool uploaded = false;

	int fd = -1;
	try {
		fd = open_file(file_path);
	} catch (filesystem_exception& e) {
		e.info();
		return;
	}

	// try to send file to servers one be one starting from one with 
	// biggest free space
	// break on success
	for (int i = 0; i < (int)Client::get().servers.size(); i++) {
		string ip = Client::get().servers[i].ip_unicast;

		// get perminssion and port
		int port = get_port_to_upload(req, ip);
		
		if (port < 0) {
			continue;
		}

		// upload file
		thread th(upload_file, ip, port, file_name, fd);
		th.detach();

		uploaded = true;
		break;	
	}

	if (!uploaded) {
		file_too_big(file_name);
	}
}


void discover_resp(Packet req) {
	validate_hello_packet(req);
	long long max_space = Server::get().MAX_SPACE;
	Packet resp("GOOD_DAY", req.cmd_seq, (max_space < 0 ? 0 : max_space), 
		Server::get().MCAST_ADDR);
	resp.addr = req.addr;

	Server::get().udp_send(resp, req.addr);
}


void search_resp(Packet req) {
	// prepare list of matching shared files
	vector<string> files;
	for (auto f : Server::get().shared_files) {
		if (f.find(req.data) != string::npos)
			files.push_back(f);
	}

	if (files.empty())
		return;
	
	// piece of files list to send
	string piece;

	Packet resp("MY_LIST", req.cmd_seq, "");
	resp.addr = req.addr;
	
	// send files without exceeding the message limit
	for (auto f : files) {
		piece = f + string("\n");
		
		// send packet if there is no space left in it
		if ((int)piece.size() > resp.get_free_data_space()) {
			if (resp.data.back() == '\n')
				resp.data.pop_back();

			Server::get().udp_send(resp, req.addr);
			resp.data = "";
		}
		 
		resp.data += piece;
	}

	if (resp.data.back() == '\n')
		resp.data.pop_back();

	Server::get().udp_send(resp, req.addr);
}


void remove_resp(Packet req) {
	validate_del_packet(req);
	string file_name = req.data;
	if (in_storage(file_name)) {
		// remove file and update list and free space
		string file_path = Server::get().SHRD_FLDR + req.data;
		uint64_t file_size = get_file_size(file_path);
		Server::get().shared_files.erase(file_name);
		Server::get().MAX_SPACE += file_size;
		remove_file(file_path);
	}
}


// upload data to client with TCP
void upload_to_client(int sock, int fd) {
	sock = tcp_accept(sock);
	if (sock < 0)
		return;
	
	try {
		tcp_send(sock, fd);
	} catch (file_transfer_exception& e) {}

	close(fd);
}

 
void fetch_resp(Packet req) {
	validate_get_packet(req);

	// prepare free socket and port
	int sock = get_free_tcp_socket(false);
	int port = get_free_tcp_port(sock);

	Packet res("CONNECT_ME", req.cmd_seq, port, req.data);
	res.addr = req.addr;

	// prepare file content
	string file_path = Server::get().SHRD_FLDR + req.data;
	int fd = -1;
	try {
		fd = open_file(file_path);
	} catch (filesystem_exception& e) {
		e.info();
		return;
	}

	// open connection
	tcp_listen(sock);

	// notify client
	Server::get().udp_send(res, req.addr);

	// upload file
	thread th(upload_to_client, sock, fd);
	th.detach();
}


// download file as a response to request req
void download_file_from_client(int sock, Packet req) {
	string data = "";
	int fd = -1;
	try {
		// transfer data
		sock = tcp_accept(sock);
		if (sock < 0)
			throw file_transfer_exception("");
		
		fd = open_file(Server::get().SHRD_FLDR + req.data);

		Server::get().tcp_receive(sock, fd);

		close(fd);

	} catch (file_transfer_exception& e) {
		Server::get().MAX_SPACE += req.param;
		Server::get().shared_files.erase(req.data);
		close(fd);
		return;
	} catch (filesystem_exception& e) {
		Server::get().MAX_SPACE += req.param;
		Server::get().shared_files.erase(req.data);
		return;
	}
}

void upload_resp(Packet req) {
	validate_add_packet(req);

	// prepare response to upload request
	uint64_t file_size = req.param;
	string file_name = req.data;
	uint64_t cmd_seq = req.cmd_seq;

	Packet res = Packet("NO_WAY", cmd_seq, file_name);
	if ((long long)file_size > Server::get().MAX_SPACE) {} 
	else if (!valid_file_name(file_name)) {} 
	else if (in_storage(file_name)) {} 
	else if (file_name.find("/") != string::npos) {} 
	else {
		res = Packet("CAN_ADD", cmd_seq, 0, "");
		// reserve space
		Server::get().MAX_SPACE -= file_size;
		Server::get().shared_files.insert(file_name);
	}

	if (res.cmd == "CAN_ADD") {
		// download file if possible
		int sock = get_free_tcp_socket(false);
		int port = get_free_tcp_port(sock);
		res.param = port;

		tcp_listen(sock);

		Server::get().udp_send(res, req.addr);
		thread th(download_file_from_client, sock, req);
		th.detach();
	} else {
		// send refusal
		Server::get().udp_send(res, req.addr);
	}
}
