/**
Citations:
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
*/

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
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

int recv(ip::tcp::socket & read_socket, ip::tcp::socket & write_socket) {
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
      if (recv(client_socket, server_socket) == -1) {
        cout << "client Ended" << endl;
        FD_CLR(client_socket.native_handle(), &rset);
        break;
      }
    }

    //Reading from server
    if (FD_ISSET(server_socket.native_handle(), &rset)) {
      cout << "Server sending" << endl;
      if (recv(server_socket, client_socket) == -1) {
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

  /**
  while (true) {
    if (recv(client_socket, server_socket) == -1) {
      break;
    }
    cout << "Outside" << endl;
    if (recv(server_socket, client_socket) == -1) {
      break;
    }
    cout << "Yep" << endl;
  }
  /**
  while (true) {
    client_socket.read_some(boost::asio::buffer(buf), error);
    if (error == boost::asio::error::eof)
      break;
    write(server_socket, boost::asio::buffer(buf));
  }
  cout << "something" << endl;
  while (true) {
    server_socket.read_some(boost::asio::buffer(buf), error);
    if (error == boost::asio::error::eof)
      break;
    write(client_socket, boost::asio::buffer(buf));
  }
  */
  cout << "About close" << endl;
  server_socket.shutdown(ip::tcp::socket::shutdown_send);
  //server_socket.close();
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
  /**

  //Will Receive new connection
  boost::asio::ip::tcp::socket socket2{ioc};

  //Wait for the connection
  acceptor.accept(socket2);

  do_session(socket2, ioc);
  **/
  cout << "Ending the server" << endl;
  logfile.close();

  return EXIT_SUCCESS;
}
