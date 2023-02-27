/**
Citations:
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
2) https://man7.org/linux/man-pages/man2/select.2.html
3) https://www.geeksforgeeks.org/tcp-and-udp-server-using-select/
*/

/**
Note: Boost Socket follow RAII design where socket is destroyed, it will be closed as-if by socket.close(ec) during the destruction of the socket.
https://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/reference/basic_io_object/_basic_io_object.html
 **/

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/bind/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <sys/select.h>
#include <time.h>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
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
  cout << str << endl;
  // look into finally call to release thread
}

void print_cache() {
  cout << "-------------------------------" << endl;
  cout << "Printing the cache!" << endl;
  int i = 1;
  for (const auto & elem : cache) {
    cout << i << ": " << elem.first << ":\n" << elem.second << "\n";
    i++;
  }
  cout << "-------------------------------" << endl;
}

void forwardRequest(http::request<http::string_body> & client_request,
                    io_context & ioc,
                    ip::tcp::socket & client_socket,
                    int ID) {
  //Setting up Host and ports
  string host = string(client_request.at("Host"));
  string port = HTTP_PORT;
  string path = parsePath(string(client_request.target()), host);
  http::request<http::string_body> forward_request = client_request;
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
  write_log(to_string(ID) + ": Received \"" +
            response_headers.substr(0, response_headers.find("\n") - 1) + "\" from " +
            host);
  http::write(client_socket, response);
  write_log(to_string(ID) + ": Responding \"" +
            response_headers.substr(0, response_headers.find("\n") - 1) + "\"");

  // if (cacheable) {
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
  if (response_headers.find("200 OK") != std::string::npos) {
    write_log(to_string(ID) + ": Tunnel closed");
  }

  cache_mtx.lock();
  // cout << "PATH: " << path << endl;
  cache.insert({host, response});
  //print_cache();
  cache_mtx.unlock();
}

// returns no bytes read or -1 incase of EOF was reached
int forwardBytes(ip::tcp::socket & read_socket, ip::tcp::socket & write_socket) {
  std::vector<char> buf(65536);
  boost::system::error_code error;

  //Reading bytes from read socket
  size_t noBytesRead = read_socket.read_some(boost::asio::buffer(buf), error);

  if (error == boost::asio::error::eof) {
    return EOF_ERROR;
  }

  //Writing bytes to write socket
  write(write_socket, boost::asio::buffer(buf, noBytesRead));

  return noBytesRead;
}

void multiplexingClientServer(ip::tcp::socket & client_socket,
                              ip::tcp::socket & server_socket,
                              int ID) {
  fd_set read_FDs;
  // Wait up to five seconds.
  struct timeval tv;
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
      if (forwardBytes(client_socket, server_socket) == EOF_ERROR) {
        break;
      }
    }

    //Reading from server socket
    if (FD_ISSET(server_socket.native_handle(), &read_FDs)) {
      if (forwardBytes(server_socket, client_socket) == EOF_ERROR) {
        break;
      }
    }

    if (nready <= 0) {
      write_log(to_string(ID) + ": WARNING No response from  5 seconds");
      break;
    }
  }
}

void forwardConnectRequest(http::request<http::string_body> & request,
                           io_context & ioc,
                           ip::tcp::socket & client_socket,
                           int ID) {
  //Setting the host
  string host = string(request.at("Host"));
  int pos = host.find(":443");
  host = host.erase(pos, pos + 4);
  string port = "443";

  //Setup Server Socket
  ip::tcp::socket server_socket = setUpSocketToConnect(host, port, ioc);

  //Send a Response 200 OK back to client
  http::response<http::string_body> response{http::status::ok, request.version()};
  http::write(client_socket, response);

  //Multiplexing client and server
  multiplexingClientServer(client_socket, server_socket, ID);
  write_log(to_string(ID) + ": Tunnel closed");
  server_socket.shutdown(ip::tcp::socket::shutdown_both);
}

void do_session(ip::tcp::socket & socket, io_context & ioc, int ID) {
  flat_buffer buff;
  char date_time_string[100];

  //Receving the request from client
  http::request<http::string_body> request;
  boost::system::error_code error;
  http::read(socket, buff, request, error);

  //Error check
  if (error == http::error::end_of_stream || error == http::error::partial_message) {
    write_log(to_string(ID) + ": WARNING " + error.message());
    socket.shutdown(ip::tcp::socket::shutdown_both);
    return;
  }

  auto t = time(nullptr);
  tm * tim = localtime(&t);
  strncpy(date_time_string, asctime(tim), strlen(asctime(tim)) - 1);
  std::string const strHeaders = boost::lexical_cast<std::string>(request.base());
  write_log(to_string(ID) + ": \"" + strHeaders.substr(0, strHeaders.find("\n") - 1) +
            "\" from " + socket.remote_endpoint().address().to_string() + " @ " +
            date_time_string);

  //Checking if it is a connect request
  if (request.method_string() == "CONNECT") {
    forwardConnectRequest(request, ioc, socket, ID);
  }
  else if (request.method_string() == "GET") {
    forwardRequest(request, ioc, socket, ID);
  }
  else if (request.method_string() == "POST") {
    forwardRequest(request, ioc, socket, ID);
  }
  else {
    write_log(to_string(ID) + ": WARNING Unknown HTTP request made");
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
  socket.shutdown(ip::tcp::socket::shutdown_both);
}

int main(int argc, char ** argv) {
  logfile.open("/var/log/erss/proxy.log");

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
  logfile.close();

  return EXIT_SUCCESS;
}
