#Makefile fuer mycommand
all: client server

client: client.cpp;
	g++ -g -Wall client.cpp -o client
		
server: server.cpp; 
	g++ -g -Wall server.cpp -o server

clean:
	rm -f client server