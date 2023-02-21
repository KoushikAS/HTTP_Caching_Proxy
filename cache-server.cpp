/**
Citations:
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
*/

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace std;
using namespace boost::beast;
using namespace boost::asio;

unordered_map<string, http::response<http::dynamic_body> > cache;

void forwardRequest(http::request<http::string_body> & request,
                    io_context & ioc,
                    ip::tcp::socket & socket) {
  string host = string(request.at("Host"));
  string port = "80";
  ip::tcp::resolver resolver(ioc);
  tcp_stream stream(ioc);

  string path = string(request.target());
  path = path.erase(0, 7);              //Removing 'http://'
  path = path.erase(0, host.length());  //Removing the host name from the url
  cout << path << endl;
  request.target(path);

  // Look up the domain name
  auto const results = resolver.resolve(host, port);
  // boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);

  // Make the connection on the IP address we get from a lookup
  stream.connect(results);

  http::write(stream, request);

  http::response<http::dynamic_body> response;
  flat_buffer buff;

  http::read(stream, buff, response);

  //Wrting response back to the client
  cout << response.base() << endl;
  http::write(socket, response);
}

std::string make_string(boost::asio::streambuf & streambuf) {
  return {boost::asio::buffers_begin(streambuf.data()),
          boost::asio::buffers_end(streambuf.data())};
}

void forwardConnectRequest(http::request<http::string_body> & request,
                           io_context & ioc,
                           ip::tcp::socket & client_socket) {
  //Setting the host
  string host = string(request.at("Host"));
  int pos = host.find(":443");
  host = host.erase(pos, pos + 4);
  string port = "443";
  ip::tcp::resolver resolver(ioc);
  tcp_stream server_socket(ioc);

  // Look up the domain name
  auto const results = resolver.resolve(host, port);
  // Make the connection on the IP address we get from a lookup
  server_socket.connect(results);

  //Sending Request Ok back to client
  http::response<http::string_body> response{http::status::ok, request.version()};
  boost::system::error_code ec;
  cout << response.base() << endl;
  http::write(client_socket, response);
  cout << response << endl;

  boost::asio::streambuf buffer;

  read(client_socket, buffer, ec);
  if (!ec || ec == boost::asio::error::eof) {
    std::cout << "Writing: " << make_string(buffer) << std::endl;
    write(server_socket, buffer);
  }

  boost::asio::streambuf buffer2;
  read(server_socket, buffer2, ec);
  if (!ec || ec == boost::asio::error::eof) {
    std::cout << "Writing: " << make_string(buffer2) << std::endl;
    write(client_socket, buffer2);
  }

  cout << "Done" << endl;
}

void do_session(ip::tcp::socket & socket, io_context & ioc) {
  flat_buffer buff;

  //Receving the request from client
  http::request<http::string_body> request;
  http::read(socket, buff, request);

  cout << request << endl;

  //Checking if it is a connect request
  if (request.method_string() == "CONNECT") {
    cout << "Inside connect call" << endl;
    forwardConnectRequest(request, ioc, socket);
  }
  else {
    cout << "Inside Get Call" << endl;
    forwardRequest(request, ioc, socket);
  }

  /**
  string host = string(request.at("Host"));

  response = forwardRequest(request, ioc);


  //Retirving from cache
  if (cache.find(host) != cache.end()) {
    cout << "Retriving from cache" << endl;
    response = cache[host];
  }
  else {
    if (request.method_string() == "CONNECT") {
      response = forwardConnectRequest(request, ioc, socket);
    }
    else {
      response = forwardRequest(request, ioc);
    }

    cache[host] = response;
  }
  **/

  socket.shutdown(ip::tcp::socket::shutdown_send);
}

int setUpSocketToListen(() {
  struct addrinfo hints;
  struct addrinfo * hosts;
  const char * port = "12345";

  memset(&hints, 0, sizeof(hints));

  hints.ai_flags = AF_INET;         // To return address family from both IPV4 and IPV6.
  hints.ai_socktype = SOCK_STREAM;  // Connection based protocol (i.e. TCP).
  hints.ai_flags = AI_PASSIVE;      // Return Socket will be suitable for bind and accept.

  int status = getaddrinfo(NULL, port, &hints, hosts);
  if (status != 0) {
    std::cerr << "Error cannot get the addresses" << std::endl;
    exit(EXIT_FAILURE);
  }

  int socket_fd =
      socket((*hosts)->ai_family, (*hosts)->ai_socktype, (*hosts)->ai_protocol);
  if (socket_fd == -1) {
    std::cerr << "Error cannot create socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (socket_fd == -1) {
    std::cerr << "Error  cannot create socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  int yes = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  struct sockaddr_in my_addr;
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = 0;
  socklen_t len = sizeof(my_addr);

  int status = bind(socket_fd, (struct sockaddr *)&my_addr, len);
  if (status == -1) {
    std::cerr << "Error cannot bind socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  status = listen(socket_fd, 100);
  if (status == -1) {
    std::cerr << "Error cannot listen on socket" << std::endl;
    exit(EXIT_FAILURE);
  }

  return socket_fd;
}



int main(int argc, char ** argv) {
  // logging
  ofstream logfile;
  logfile.open("/var/log/erss/proxy.log");
  logfile << "Started the server" << endl;

  int socket = setUpSocketToListen();

  do_session(socket);

  cout << "Ending the server" << endl;
  logfile.close();

  return EXIT_SUCCESS;
}
