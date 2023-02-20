/**
Citations:
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
*/

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
using namespace std;

boost::beast::http::response<boost::beast::http::dynamic_body> forwardRequest(
    boost::beast::http::request<boost::beast::http::string_body> & request,
    boost::asio::io_context & ioc) {
  std::string host = std::string(request.at("Host"));
  std::string port = "80";
  boost::asio::ip::tcp::resolver resolver(ioc);
  boost::beast::tcp_stream stream(ioc);

  // Look up the domain name
  auto const results = resolver.resolve(host, port);
  // boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);

  // Make the connection on the IP address we get from a lookup
  stream.connect(results);

  boost::beast::http::write(stream, request);

  boost::beast::http::response<boost::beast::http::dynamic_body> response;
  boost::beast::flat_buffer buff;

  boost::beast::http::read(stream, buff, response);
  return response;
}

void do_session(boost::asio::ip::tcp::socket & socket, boost::asio::io_context & ioc) {
  boost::beast::flat_buffer buff;

  boost::beast::http::request<boost::beast::http::string_body> request;
  cout << "here" << endl;
  boost::beast::http::read(socket, buff, request);

  cout << request << endl;
  // boost::beast::http::response<boost::beast::http::dynamic_body> response = NULL;
  boost::beast::http::response<boost::beast::http::dynamic_body> response =
      forwardRequest(request, ioc);
  boost::beast::http::write(socket, response);

  socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
}

int main(int argc, char ** argv) {
  // logging
  ofstream logfile;
  logfile.open("/var/log/erss/proxy.log");
  logfile << "Started the server" << endl;

  // boost::asio::ip::address addr = boost::asio::ip::make_address("127.0.0.1");
  boost::asio::ip::address addr = boost::asio::ip::make_address("0.0.0.0");
  unsigned short port_num = 12345;

  boost::asio::io_context ioc{1};

  boost::asio::ip::tcp::acceptor acceptor{ioc, {addr, port_num}};

  //Will Receive new connection
  boost::asio::ip::tcp::socket socket{ioc};
  
  cout << "Waiting for connection at " << endl;
  //Wait for the connection
  acceptor.accept(socket);
  logfile << "got to here" << endl;
  
  do_session(socket, ioc);
  cout << "Ending the server" << endl;
  logfile.close();

  return EXIT_SUCCESS;
}
