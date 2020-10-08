#include "parser.h"
#include "client.h"
#include "display.h"
#include "packet.h"
#include "transfer.h"
#include "files.h"
#include "validation.h"
#include "command.h"


int main(int argc, char** argv) {
	// try to parse commmand line arguments
	try {
		parse_args_client(argc, argv);
	} catch (po::error& e) {
		cout << e.what() << endl;
		return 1;
	} catch (options_exception& e) {
		e.info();
		return 1;
	}

	// try to init socket
	try {
		Client::get().init_sock();
	} catch (socket_exception& e) {
		e.info();
		return 1;
	}

	string line;

	while (1) {
		// read and parse line
		getline(cin, line);
		ParsingResult res = parse_line(line);

		// execute proper command
		if (res.cmd == "discover") discover();
		else if (res.cmd == "search") search(res.arg);
		else if (res.cmd == "fetch") fetch(res.arg);
		else if (res.cmd == "upload") upload(res.arg);
		else if (res.cmd == "remove") remove_cmd(res.arg);
		else if (res.cmd == "exit") break;
	}

	return 0;
}