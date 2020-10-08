#pragma once 

#include <vector>
#include <iostream>

// Tools for working with files

// Get names of file from folder specified by folder_path (not recursive)
std::vector<std::string> get_file_names(std::string folder_path);

void remove_file(std::string file_path);

// Returns string of bytes from file
std::string get_file_content(std::string file_path);

void save_file(std::string file_name, std::string content);

// Extracts file name from its path
std::string get_file_name(std::string file_path);

// Checks if server has file_name in storage
bool in_storage(std::string file_name);

uint64_t get_file_size(std::string file_path);

bool file_exists(std::string file_path);

// open or create file with a given file_path and return its file descriptor
int open_file(std::string file_path);