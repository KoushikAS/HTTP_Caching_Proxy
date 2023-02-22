/**
Citations:
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
*/

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <sys/select.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace std;
using namespace boost::beast;
using namespace boost::asio;

unordered_map<string, http::response<http::dynamic_body> > cache;

const string HTTP_PORT = "80";

string parsePath(string path, string host) {
  path = path.erase(0, 7);              //Removing 'http://'
  path = path.erase(0, host.length());  //Removing the host name from the url
  return path;
}

ip::tcp::resolver::results_type findAddress(string host, string port, io_context & ioc) {
  ip::tcp::resolver resolver(ioc);
  return resolver.resolve(host, port);
}

ip::tcp::socket setUpSocketToConnect(string host, string port, io_context & ioc) {
  ip::tcp::socket socket(ioc);

  // Look up the domain name
  auto const results = findAddress(host, port, ioc);

  // Make the connection on the IP address we get from a lookup
  socket.connect(*results);

  return socket;
}

void forwardRequest(http::request<http::string_body> & client_request,
                    io_context & ioc,
                    ip::tcp::socket & client_socket) {
  //Setting up Host and ports
  string host = string(client_request.at("Host"));
  string port = HTTP_PORT;
  string path = parsePath(string(client_request.target()), host);

  ip::tcp::socket server_socket = setUpSocketToConnect(host, port, ioc);

  //Setting up the request body to send to server
  http::request<http::string_body> forward_request = client_request;
  forward_request.target(path);

  cout << forwardRequest << endl;

  //Forward Request to Server
  http::write(server_socket, forward_request);

  http::response<http::dynamic_body> response;
  flat_buffer buff;

  //Relay back the response from server to client
  http::read(server_socket, buff, response);
  http::write(client_socket, response);

  cout << response.base() << endl;
}

int forwardBytes(ip::tcp::socket & read_socket, ip::tcp::socket & write_socket) {
  boost::array<char, 1024> buf;

  boost::system::error_code error;
  size_t len = read_socket.read_some(boost::asio::buffer(buf), error);
  std::cout << " read " << len << " bytes"
            << std::endl;  // called multiple times for debugging!

  if (error == boost::asio::error::eof)
    return -1;
  else if (error)
    throw boost::system::system_error(error);  // Some other error.

  write(write_socket, boost::asio::buffer(buf));

  cout << "Outside" << endl;
  return 1;
}

void forwardConnectRequest(http::request<http::string_body> & request,
                           io_context & ioc,
                           ip::tcp::socket & client_socket) {
  //Setting the host
  string host = string(request.at("Host"));
  int pos = host.find(":443");
  host = host.erase(pos, pos + 4);
  string port = "443";

  cout << "host" << endl;
  cout << host << endl;
  cout << "port" << endl;
  cout << port << endl;
  ip::tcp::resolver resolver(ioc);
  ip::tcp::socket server_socket(ioc);

  // Look up the domain name
  auto const results = resolver.resolve(host, port);
  // Make the connection on the IP address we get from a lookup
  server_socket.connect(*results);

  //Sending Request Ok back to client
  http::response<http::string_body> response{http::status::ok, request.version()};
  boost::system::error_code ec;
  cout << response.base() << endl;
  http::write(client_socket, response);
  cout << response << endl;

  // Mutliplexing both Client and Server port
  int maxFd = max(server_socket.native_handle(), client_socket.native_handle()) + 1;
  fd_set rset;
  struct timeval tv;
  // Wait up to five seconds.
  tv.tv_sec = 5;
  tv.tv_usec = 0;

  while (true) {
    FD_SET(server_socket.native_handle(), &rset);
    FD_SET(client_socket.native_handle(), &rset);

    int nready = select(maxFd, &rset, NULL, NULL, &tv);
    cout << nready << endl;

    //Reading from client
    if (FD_ISSET(client_socket.native_handle(), &rset)) {
      cout << "Client sending" << endl;
      if (forwardBytes(client_socket, server_socket) == -1) {
        cout << "client Ended" << endl;
        FD_CLR(client_socket.native_handle(), &rset);
        break;
      }
    }

    //Reading from server
    if (FD_ISSET(server_socket.native_handle(), &rset)) {
      cout << "Server sending" << endl;
      if (forwardBytes(server_socket, client_socket) == -1) {
        cout << "server ended" << endl;
        FD_CLR(server_socket.native_handle(), &rset);
        break;
      }
    }

    if (nready <= 0) {
      cout << "no data from 5 sec" << endl;
      break;
    }
  }

  cout << "About close" << endl;
  server_socket.shutdown(ip::tcp::socket::shutdown_send);
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

int main(int argc, char ** argv) {
  // logging
  ofstream logfile;
  logfile.open("/var/log/erss/proxy.log");
  logfile << "Started the server" << endl;

  // boost::asio::ip::address addr = boost::asio::ip::make_address("127.0.0.1");
  ip::address addr = ip::make_address("0.0.0.0");
  unsigned short port_num = 12345;

  io_context ioc{1};
  //Listen to new connection
  ip::tcp::acceptor acceptor{ioc, {addr, port_num}};

  while (true) {
    //Socket creation
    ip::tcp::socket socket{ioc};

    cout << "Waiting for connection at " << endl;
    //Wait for the connection
    acceptor.accept(socket);

    do_session(socket, ioc);
  }
  cout << "Ending the server" << endl;
  logfile.close();

  return EXIT_SUCCESS;
}
