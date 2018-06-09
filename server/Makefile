server : main.o server.o 
	g++ -pthread -o server main.o server.o 
main.o : main.cpp header.h
	g++ -c -pthread main.cpp
server.o : server.cpp header.h
	g++ -c -g -Wall -pthread server.cpp 
clean : 
	rm server main.o server.o  
