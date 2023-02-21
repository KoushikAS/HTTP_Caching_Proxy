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

unordered_map<std::string,
              boost::beast::http::response<boost::beast::http::dynamic_body> >
    cache;

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

boost::beast::http::response<boost::beast::http::dynamic_body> forwardConnectRequest(
    boost::beast::http::request<boost::beast::http::string_body> & request,
    boost::asio::io_context & ioc,
    boost::asio::ip::tcp::socket & client_socket) {
  boost::beast::http::request<boost::beast::http::string_body> forwardRequest;

  std::string host = std::string(request.at("Host"));
  int pos = host.find(":443");
  host = host.erase(pos, pos + 4);
  forwardRequest.set(boost::beast::http::field::host, host);

  std::string url = std::string(request.target());
  cout << url << endl;

  pos = url.find(":443");
  url = url.erase(pos, pos + 4);
  url = url.append("/");
  cout << url << endl;
  forwardRequest.target("/");

  forwardRequest.method(boost::beast::http::verb::get);

  forwardRequest.version(request.version());

  //std::string host = "www.google.com";
  //request.set(boost::beast::http::field::host, host);

  std::string port = "443";
  boost::asio::ip::tcp::resolver resolver(ioc);

  boost::asio::ssl::context ssl_context(boost::asio::ssl::context::sslv23_client);
  //ssl_context.set_default_verify_paths();

  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssocket = {ioc, ssl_context};

  auto results = resolver.resolve(host, port);

  connect(ssocket.lowest_layer(), results);
  ssocket.handshake(boost::asio::ssl::stream_base::handshake_type::client);

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
  boost::beast::http::write(ssocket, forwardRequest);

  boost::beast::http::response<boost::beast::http::dynamic_body> response;
  boost::beast::flat_buffer buff;

  boost::beast::http::read(ssocket, buff, response);

  return response;
}

void do_session(boost::asio::ip::tcp::socket & socket, boost::asio::io_context & ioc) {
  boost::beast::flat_buffer buff;

  boost::beast::http::request<boost::beast::http::string_body> request;

  boost::beast::http::read(socket, buff, request);

  cout << request << endl;

  boost::beast::http::response<boost::beast::http::dynamic_body> response;

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
