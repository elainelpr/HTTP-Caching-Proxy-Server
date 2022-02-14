main: main1.o Client.o Server.o Socket.o
	g++ -o main1 main1.o Client.o Server.o Socket.o
main.o: main1.cpp
	g++ $(CPPFLAGS) -c main1.cpp
Socket.o: Socket.cpp Socket.hpp
	g++ $(CPPFLAGS) -c Socket.cpp
Server.o: Server.cpp Server.hpp
	g++ $(CPPFLAGS) -c Server.cpp
Client.o: Client.cpp Client.hpp
	g++ $(CPPFLAGS) -c Client.cpp
.PHONY: clean
clean:
	rm -f *.o *~ main1
