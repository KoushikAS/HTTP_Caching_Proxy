TARGETS= cache-server

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

cache-server: cache-server.cpp
	g++ -g -o $@ $<