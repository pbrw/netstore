#include <set>
#include <unistd.h>
#include <experimental/filesystem>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "files.h"
#include "display.h"
#include "server.h"

using namespace std;
namespace fs = experimental::filesystem;

vector<string> get_file_names(string folder_path) {
	vector<string> res;
	for (const auto & entry : fs::directory_iterator(folder_path)) {
		if (fs::is_regular_file(fs::status(entry))) {
			res.push_back(entry.path().filename());
		}
	}

	return res;
}


void remove_file(string file_path) {
	fs::remove(file_path);
}


string get_file_content(string file_path) {
	ifstream ifs(file_path, ios::binary);
	string content;
	content.assign((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));

	return content;
}


void save_file(string file_path, string content) {
	ofstream file(file_path, ios::binary);
	file << content;
	file.close();
}


string get_file_name(string file_path) {
	fs::path p = file_path;
	return p.filename();
}


bool in_storage(string file_name) {
	set<string>* ptr = &Server::get().shared_files;
	return ptr->find(file_name) != ptr->end();
}


uint64_t get_file_size(string file_path) {
	fs::path p(file_path);
	return fs::file_size(p);
}


bool file_exists(string file_path) {
	return access(file_path.c_str(), F_OK) == 0;
}

int open_file(string file_path) {
	int res = open(file_path.c_str(), (O_RDWR | O_CREAT), S_IRWXU);
	if (res < 0) {
		throw filesystem_exception("Could not open or create a file");
	}
	return res;
}	