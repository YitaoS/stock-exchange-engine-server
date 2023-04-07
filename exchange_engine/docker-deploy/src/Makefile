###complie option###
CC = g++
DREW_OF_THREE = -Wall -Werror -pedantic 
CFLAGS = -std=c++11 -MMD  $(MTHREAD_FLAG) -lpqxx -lpq -lpugixml# $(INCLUDE_DIR)

MTHREAD_FLAG = -pthread -fsanitize=thread
# INCLUDE_DIR = -I /user/include/boost
LIBS = -lssl -lcrypto 

###
all: engine
engine: engine.o stock_server.o session.o db_interact.o parser.o handle_command.o
	$(CC) $(CFLAGS) $^ -o $@ 

engine.o:engine.cpp stock_server.hpp db_interact.hpp
	$(CC) $(CFLAGS) -c $< -o $@ 

stock_server.o:stock_server.cpp stock_server.hpp session.hpp
	$(CC) $(CFLAGS) -c $< -o $@

session.o:session.cpp session.hpp db_interact.hpp handle_command.hpp
	$(CC) $(CFLAGS) -c $< -o $@

db_interact.o:db_interact.cpp db_interact.hpp
	$(CC) $(CFLAGS) -c $< -o $@

parser.o:parser.cpp parser.hpp command.hpp
	$(CC) $(CFLAGS) -c $< -o $@

handle_command.o:handle_command.cpp handle_command.hpp
	$(CC) $(CFLAGS) -c $< -o $@


test: test.cpp parser.hpp parsed_create.hpp parsed_transcation.hpp parser.cpp command.hpp parsed_create.hpp parsed_transcation.hpp
	$(CC) $(CFLAGS) $^ -o $@ 
-include $(wildcard *.d)

.PHONY:
clean:
	rm -rf *~ *.o *.d engine 