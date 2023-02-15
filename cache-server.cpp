/**
Citations:
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
*/

#include<iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>


int exitWithErrorMessage(std::string msg){
  std::cerr<<msg<<std::endl;
  return EXIT_FAILURE;
}

int main(int argc, char **argv){
  const char *hostname = NULL;
  const char *port     = "12345";
  struct addrinfo hints;
  struct addrinfo *hosts;
  
  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;

  int status = getaddrinfo(hostname, port, &hints, &hosts);

  if(status != 0) {
    return exitWithErrorMessage("Error: was not able to get the address info for the host");
  }

  int socket_fd = socket(hosts->ai_family, hosts->ai_socktype, hosts->ai_protocol);

  if(socket_fd == -1){
    return exitWithErrorMessage("Error: Cannot open a socket");
  }

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  status = bind(socket_fd, hosts->ai_addr, hosts->ai_addrlen);

  if(status == -1){
    return exitWithErrorMessage("Error while binding the socket");
  }

  status = listen(socket_fd, 100);

  if(status == -1){
    return exitWithErrorMessage("Error cannot listen to the socket");
  }

  std::cout<< "Waiting for connection"<<std::endl;
  int client_connection_fd = accept(socket_fd, NULL, NULL);
  
  if(client_connection_fd == -1){
    return exitWithErrorMessage("Error cannot accept the incoming request");
  }

  char buffer[512];
  recv(client_connection_fd, buffer, 512, 0);
  std::cout<<"Received "<<buffer<<std::endl;

  freeaddrinfo(hosts);
  close(socket_fd);
  return EXIT_SUCCESS;
}