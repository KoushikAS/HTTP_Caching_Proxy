/**
Citations:
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
*/

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

http::response<http::dynamic_body> forwardRequest(
    http::request<http::string_body> & request,
    io_context & ioc) {
  string host = string(request.at("Host"));
  string port = "80";
  ip::tcp::resolver resolver(ioc);
  tcp_stream stream(ioc);

  // Look up the domain name
  auto const results = resolver.resolve(host, port);
  // boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);

  // Make the connection on the IP address we get from a lookup
  stream.connect(results);

  http::write(stream, request);

  http::response<http::dynamic_body> response;
  flat_buffer buff;

  http::read(stream, buff, response);

  return response;
}

http::response<http::dynamic_body> forwardConnectRequest(
    http::request<http::string_body> & request,
    io_context & ioc,
    ip::tcp::socket & client_socket) {
  http::request<http::string_body> forwardRequest;

  string host = string(request.at("Host"));
  int pos = host.find(":443");
  host = host.erase(pos, pos + 4);
  forwardRequest.set(http::field::host, host);

  string url = string(request.target());
  cout << url << endl;

  pos = url.find(":443");
  url = url.erase(pos, pos + 4);
  url = url.append("/");
  cout << url << endl;
  forwardRequest.target("/");

  forwardRequest.method(http::verb::get);

  forwardRequest.version(request.version());

  //std::string host = "www.google.com";
  //request.set(boost::beast::http::field::host, host);

  string port = "443";
  ip::tcp::resolver resolver(ioc);

  ssl::context ssl_context(ssl::context::sslv23_client);
  //ssl_context.set_default_verify_paths();

  ssl::stream<ip::tcp::socket> ssocket = {ioc, ssl_context};

  auto results = resolver.resolve(host, port);

  connect(ssocket.lowest_layer(), results);
  ssocket.handshake(ssl::stream_base::handshake_type::client);

  //Sending Request Ok back to client
  // boost::beast::http::response<boost::beast::http::string_body> response{
  //   boost::beast::http::status::ok, request.version()};
  //cout << response << endl;
  //boost::beast::http::write(client_socket, response);

  const string path = "/";

  cout << request << endl;

  /**
  boost::beast::http::request<boost::beast::http::string_body> req{
      boost::beast::http::verb::get, path, 11};
  req.set(boost::beast::http::field::host, host);

  boost::beast::http::write(ssocket, req);
  **/
  cout << forwardRequest << endl;
  http::write(ssocket, forwardRequest);

  http::response<http::dynamic_body> response;
  flat_buffer buff;

  http::read(ssocket, buff, response);

  return response;
}

void do_session(ip::tcp::socket & socket, io_context & ioc) {
  flat_buffer buff;

  http::request<http::string_body> request;

  http::read(socket, buff, request);

  cout << request << endl;

  http::response<http::dynamic_body> response;

  /**
  std::string host = request.at("Host");

  if (cache.find(host) != cache.end()) {
    std::cout << "Retriving from cache" << endl;
    response = cache[host];
  }
  else {
  **/
  //const boost::string_view CONNECT("CONNECT");
  if (request.method_string() == "CONNECT") {
    response = forwardConnectRequest(request, ioc, socket);
  }
  else {
    response = forwardRequest(request, ioc);
  }
  /**
    cache[host] = response;
  }
    **/

  cout << response.base() << endl;
  http::write(socket, response);
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

  ip::tcp::acceptor acceptor{ioc, {addr, port_num}};

  //Will Receive new connection
  ip::tcp::socket socket{ioc};

  cout << "Waiting for connection at " << endl;
  //Wait for the connection
  acceptor.accept(socket);
  logfile << "got to here" << endl;

  do_session(socket, ioc);

  /** Second call
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
