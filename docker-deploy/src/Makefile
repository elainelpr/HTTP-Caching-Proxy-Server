CPPFLAGS = -std=c++11

main: main.o proxy.o response.o request.o cache.o
	g++ -o main main.o proxy.o response.o request.o cache.o -lpthread
main.o: main.cpp
	g++ $(CPPFLAGS) -g3 -O0 -c main.cpp
cache.o: cache.cpp cache.h
	g++ $(CPPFLAGS) -g3 -O0 -c cache.cpp
request.o: request.cpp request.h
	g++ $(CPPFLAGS) -g3 -O0 -c request.cpp
response.o: response.cpp response.h
	g++ $(CPPFLAGS) -g3 -O0 -c response.cpp
proxy.o: proxy.cpp proxy.hpp
	g++ $(CPPFLAGS) -g3 -O0 -c proxy.cpp
.PHONY: clean
clean:
	rm -f *.o *~ main
