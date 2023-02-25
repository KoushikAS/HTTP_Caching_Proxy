/**
Citations:
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
?????? https://www.boost.org/doc/libs/1_77_0/doc/html/boost_asio/example/cpp11/fork/daemon.cpp
*/

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include <sys/select.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <time.h>
#include <regex>
#include <unordered_map>

using namespace std;
using namespace boost::beast;
using namespace boost::asio;

#define BOOST_BIND_GLOBAL_PLACEHOLDERS
const string HTTP_PORT = "80";
const int EOF_ERROR = -1;
std::mutex log_mtx;
std::mutex cache_mtx;
ofstream logfile;

unordered_map<string, http::response<http::dynamic_body> > cache;

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

void write_log(string str) {
  const std::lock_guard<std::mutex> lock(log_mtx);
  logfile << str << endl;
  // look into finally call to release thread
}

void print_cache() {
  cout << "-------------------------------" << endl;
  cout << "Printing the cache!" << endl;
  int i = 1;
  for(const auto& elem : cache) {
    cout << i << ": " << elem.first << ":\n" << elem.second << "\n";
    i++;
  }
  cout << "-------------------------------" << endl;
}

void forwardRequest(http::request<http::string_body> & client_request,
                    io_context & ioc,
                    ip::tcp::socket & client_socket, int ID) {
  //Setting up Host and ports
  string host = string(client_request.at("Host"));
  string port = HTTP_PORT;
  string path = parsePath(string(client_request.target()), host);
  http::request<http::string_body> forward_request = client_request;
  std::regex re(".*\b(Cache)\b(.*)");
  std::smatch m;
  // cout << forward_request.base() << endl;
  // if (client_request.method_string() == "GET") {
  //   if (value in cache) {
  //     if (value in cache but expired) {
  //       write_log(to_string(ID) + ": in cache, but expired at (EXPIREDTIME)");
  //     }
  //     else if (in cache, requires validation) {
  //       write_log(to_string(ID) + ": in cache, requires validation");
  //     }
  //     else if (in cache, valid) {
  //       write_log(to_string(ID) + ": in cache, valid");
  //     }
  //   }
  //   else {
  //     write_log(to_string(ID) + ": not in cache");
  //     std::string const strHeaders = boost::lexical_cast<std::string>(forward_request.base());
  //     write_log(to_string(ID) + ": Requesting \"" + strHeaders.substr(0, strHeaders.find("\n")-1) + "\" from " + host);
  //   }
  //   logfile << "GET request at" << host << endl;
  //   write_log("GET request at " + host);
  // }

  ip::tcp::socket server_socket = setUpSocketToConnect(host, port, ioc);

  //Setting up the request body to send to server
  forward_request.target(path);

  // cout << forwardRequest << endl;

  //Forward Request to Server
  http::write(server_socket, forward_request);

  http::response<http::dynamic_body> response;
  flat_buffer buff;

  //Relay back the response from server to client
  http::read(server_socket, buff, response);
  std::string const response_headers = boost::lexical_cast<std::string>(response.base());
  write_log(to_string(ID) + ": Received \"" + response_headers.substr(0, response_headers.find("\n")-1) + "\" from " + host);
  http::write(client_socket, response);
  write_log(to_string(ID) + ": Responding \"" + response_headers.substr(0, response_headers.find("\n")-1) + "\"");
  cout << regex_match(response_headers, re) << " : " << response_headers << endl;
  if (regex_match(response_headers, re)) {
    regex_search(response_headers, m, re);
    write_log(to_string(ID) + m.suffix().str());
  }

  // if (response.find("Cache-Control: ") != std::string::npos || response.find("Expires: ") != std::string::npos) {
  //   if (expires) {
  //     write_log(to_string(ID) + ": not cacheable because " + "(REASON)");
  //   }
  //   else if (cached but expires) {
  //     write_log(to_string(ID) + ": cached, but expires at " + "(EXPIRES)");
  //   }
  // }
  // else {
  //   write_log(to_string(ID) + ": cached, but requires re-validation ");
  // }

  // if (response_headers.find("200 OK") != std::string::npos) {
  //   write_log(to_string(ID) + ": Tunnel closed"); 
  // }

  const std::lock_guard<std::mutex> lock(cache_mtx);
  cache.insert({string(client_request.target()), response});
  // print_cache();
  // cout << response.base() << endl;
}

// returns no bytes read or -1 incase of EOF was reached
int forwardBytes(ip::tcp::socket & read_socket, ip::tcp::socket & write_socket) {
  //boost::array<char, 65536> buf;
  std::vector<char> buf(65536);
  boost::system::error_code error;

  //Reading bytes from read socket
  size_t noBytesRead = read_socket.read_some(boost::asio::buffer(buf), error);
  // std::cout << " read " << noBytesRead << " bytes" << std::endl;

  if (error == boost::asio::error::eof) {
    return EOF_ERROR;
  }

  //Writing bytes to write socket
  write(write_socket, boost::asio::buffer(buf, noBytesRead));

  return noBytesRead;
}

void multiplexingClientServer(ip::tcp::socket & client_socket,
                              ip::tcp::socket & server_socket) {
  fd_set read_FDs;
  struct timeval tv;
  // Wait up to five seconds.
  tv.tv_sec = 5;
  tv.tv_usec = 0;

  // Mutliplexing both Client and Server port
  int maxFd = max(server_socket.native_handle(), client_socket.native_handle()) + 1;

  while (true) {
    //Setting client socket and server socket to Read File Descriptors
    FD_ZERO(&read_FDs);
    FD_SET(server_socket.native_handle(), &read_FDs);
    FD_SET(client_socket.native_handle(), &read_FDs);
    //Listening to file Read File Descriptors
    int nready = select(maxFd, &read_FDs, NULL, NULL, &tv);

    //Reading from client socket
    if (FD_ISSET(client_socket.native_handle(), &read_FDs)) {
      // cout << "Client sending" << endl;
      if (forwardBytes(client_socket, server_socket) == EOF_ERROR) {
        cout << "client Ended connection" << endl;
        break;
      }
    }

    //Reading from server socket
    if (FD_ISSET(server_socket.native_handle(), &read_FDs)) {
      // cout << "Server sending" << endl;
      if (forwardBytes(server_socket, client_socket) == EOF_ERROR) {
        cout << "server ended connection" << endl;
        break;
      }
    }

    if (nready <= 0) {
      cout << "no data from 5 sec" << endl;
      break;
    }
  }
}

void forwardConnectRequest(http::request<http::string_body> & request,
                           io_context & ioc,
                           ip::tcp::socket & client_socket, int ID) {
  //Setting the host
  string host = string(request.at("Host"));
  int pos = host.find(":443");
  host = host.erase(pos, pos + 4);
  string port = "443";

  // std::cout << host << std::endl;
  //Setup Server Socket
  ip::tcp::socket server_socket = setUpSocketToConnect(host, port, ioc);

  //Send a Response 200 OK back to client
  http::response<http::string_body> response{http::status::ok, request.version()};
  http::write(client_socket, response);

  //Multiplexing client and server
  multiplexingClientServer(client_socket, server_socket);
  write_log(to_string(ID) + ": Tunnel closed");
  server_socket.shutdown(ip::tcp::socket::shutdown_send);
}

void do_session(ip::tcp::socket & socket, io_context & ioc, int ID) {
  flat_buffer buff;
  char date_time_string[100];

  //Receving the request from client
  http::request<http::string_body> request;
  http::read(socket, buff, request);
  auto t = time(nullptr);
  tm* tim = localtime(&t);
  strncpy(date_time_string, asctime(tim), strlen(asctime(tim)) - 1);
  std::string const strHeaders = boost::lexical_cast<std::string>(request.base());
  write_log(to_string(ID) + ": \"" + strHeaders.substr(0, strHeaders.find("\n")-1) + "\" from " + socket.remote_endpoint().address().to_string() + " @ " + date_time_string);

  //Checking if it is a connect request
  if (request.method_string() == "CONNECT") {
    forwardConnectRequest(request, ioc, socket, ID);
  }
  else if (request.method_string() == "GET") {
    cout << "Inside a GET Call" << endl;
    forwardRequest(request, ioc, socket, ID);
  }
  else if (request.method_string() == "POST") {
    cout << "Inside a POST Call" << endl;
    forwardRequest(request, ioc, socket, ID);
  }
  else {
    cout << "Unknown HTTP request made" << endl;
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
  delete &socket;
}

int main(int argc, char ** argv) {
  logfile.open("/var/log/erss/proxy.log");

  // boost::asio::ip::address addr = boost::asio::ip::make_address("127.0.0.1");
  ip::address addr = ip::make_address("0.0.0.0");
  unsigned short port_num = 12345;

  io_context ioc{1};
  //Listen to new connection
  ip::tcp::acceptor acceptor{ioc, {addr, port_num}};
  int ID = 100;

  while (true) {
    // make a new socket for the client
    ip::tcp::socket * socketio = new ip::tcp::socket{ioc};

    //Blocks while waiting for a connection
    acceptor.accept(*socketio);
    ID++;
    std::thread t{do_session, std::ref(*socketio), std::ref(ioc), ID};

    t.detach();
  }
  //catch the sigabrt signal to do this when cleaning up the code (can't catch sigkill)
  cout << "Ending the server" << endl;
  logfile.close();

  return EXIT_SUCCESS;
}
