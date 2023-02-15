TARGETS= cache-server

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

cache-server: cache-server.cpp
	g++ -std=c++11 -o $@ $<