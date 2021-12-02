all: client server

client: client.o transleep.o
	g++ client.o transleep.o -o client
	
server: server.o transleep.o
	g++ server.o transleep.o -o server
	
server.o: server.cpp transleep.hpp
	g++ server.cpp -o server.o -c -Wall
	
client.o: client.cpp transleep.hpp
	g++ client.cpp -o client.o -c -Wall
	
transleep.o: transleep.cpp transleep.hpp
	g++ transleep.cpp -o transleep.o -c -Wall
	
clean:
	@rm -f client.o client server.o server transleep.o
