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

/**
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


  const string path = "/";

  cout << request << endl;

  /**
  boost::beast::http::request<boost::beast::http::string_body> req{
      boost::beast::http::verb::get, path, 11};
  req.set(boost::beast::http::field::host, host);

  boost::beast::http::write(ssocket, req);
  **/

/**
  cout << forwardRequest << endl;
  http::write(ssocket, forwardRequest);

  http::response<http::dynamic_body> response;
  flat_buffer buff;

  http::read(ssocket, buff, response);

  return response;
}
  **/

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

  //Socket creation
  ip::tcp::socket socket{ioc};

  cout << "Waiting for connection at " << endl;
  //Wait for the connection
  acceptor.accept(socket);

  do_session(socket, ioc);

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
