TARGETS= cache-server

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

cache-server: cache-server.cpp
	g++ -I ../tmp/boost_1_81_0/ -std=c++11 -o $@ $< 
