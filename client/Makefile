client : main.o client.o
	g++ -pthread -o client main.o client.o
main.o : main.cpp header.h
	g++ -c -pthread main.cpp
client.o : client.cpp header.h
	g++ -c -g -Wall -pthread client.cpp
clean : 
	rm client main.o client.o 
