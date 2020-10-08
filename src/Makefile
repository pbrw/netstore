TARGETS = netstore-server netstore-client

CXX     = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17 -lstdc++fs
LFLAGS = $(CXXFLAGS) -lboost_program_options -lpthread

all: $(TARGETS)

validation.o: validation.h packet.h files.h client.h display.h

serverData.o: serverData.h packet.h

files.o: files.h display.h server.h

transfer.o: transfer.h server.h display.h client.h packet.h

command.o: command.h server.h transfer.h client.h files.h\
	serverData.h packet.h display.h validation.h

display.o: display.h packet.h client.h serverData.h

packet.o: packet.h validation.h display.h command.h

server.o: server.h transfer.h packet.h display.h

client.o: client.h transfer.h serverData.h display.h

parser.o: parser.h server.h client.h files.h validation.h

netstore-server.o: server.h command.h parser.h display.h\
	packet.h transfer.h 

netstore-server: command.o netstore-server.o parser.o server.o\
	display.o packet.o transfer.o client.o files.o serverData.o\
	validation.o
	$(CXX) -o $@ $^ $(LFLAGS)

netstore-client.o: client.h parser.h display.h\
	packet.h command.h transfer.h files.h validation.h

netstore-client: netstore-client.o parser.o server.o client.o\
	display.o packet.o transfer.o client.o command.o files.o serverData.o\
	validation.o
	$(CXX) -o $@ $^ $(LFLAGS)

.PHONY: clean

clean:
	rm -f $(TARGETS) *.o *~ *.bak