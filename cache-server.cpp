/**
Citations:
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
2) https://man7.org/linux/man-pages/man2/select.2.html
3) https://www.geeksforgeeks.org/tcp-and-udp-server-using-select/
*/

/**
Note: Boost Socket follow RAII design where socket is destroyed, it will be closed as-if by socket.close(ec) during the destruction of the socket.
https://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/reference/basic_io_object/_basic_io_object.html
https://stackoverflow.com/questions/866822/why-both-no-cache-and-no-store-should-be-used-in-http-response
https://www.rfc-editor.org/rfc/rfc7234#section-5.2.1.5
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
#include <regex>
#include <unordered_map>
#include <utility>

using namespace std;
using namespace boost::beast;
using namespace boost::asio;

#define BOOST_BIND_GLOBAL_PLACEHOLDERS
const string HTTP_PORT = "80";
const int EOF_ERROR = -1;
std::mutex log_mtx;
std::mutex cache_mtx;
ofstream logfile;

struct cache_entry {
  time_t res_time;
  time_t exp_time;
  http::response<http::dynamic_body> res_body;
};

unordered_map<string, cache_entry> cache;

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
}

void print_cache() {
  cout << "-------------------------------" << endl;
  cout << "Printing the cache!" << endl;
  int i = 1;
  for (const auto & elem : cache) {
    cout << i << ": " << elem.first << ":\n" << asctime(localtime(&elem.second.res_time)) << asctime(localtime(&elem.second.exp_time)) << elem.second.res_body << "\n";
    i++;
  }
  cout << "-------------------------------" << endl;
}

bool check_cache(int ID, string req, string host, int max_age, int max_stale, int min_fresh) {
  const std::lock_guard<std::mutex> lock(cache_mtx); // might need to change this to not lock for entire scope
  if (cache.find(req) != cache.end()) {
    cache_entry cache_hit = cache.find(req)->second;
    std::string const response_header =
        boost::lexical_cast<std::string>(cache_hit.res_body.base());
    regex rg_cache("(Cache\\-Control\\: (.*))");
    smatch match;
    if (regex_search(response_header, match, rg_cache)) {
      if (match[2].str().find("no-cache") != string::npos) {
        write_log(to_string(ID) + ": in cache, requires validation");
      }
      else if (match[2].str().find("max-age") != string::npos) {
        regex rgx_exp("(max\\-age\\=([0-9]+))");
        smatch exp_match;
        string const temp = match[2].str();
        regex_search(temp, exp_match, rgx_exp);
        int entry_max_age = stoi(exp_match[2].str());
        if (entry_max_age == 0) {
          write_log(to_string(ID) + ": in cache, requires validation");
        } else {
          // add logic for client max-fresh
          time_t now = time(NULL);
          int expire_time = cache_hit.exp_time; // server expire time
          if (max_age != -1) {
            if (cache_hit.res_time + max_age < expire_time) { // override if the clients max age is less
              expire_time = cache_hit.res_time + max_age;
              write_log(to_string(ID) + ": NOTE=using client max-age over cache max-age");
            }
          }

          if (difftime(now + min_fresh, expire_time) <= 0) {
            write_log(to_string(ID) + ": in cache, valid");
            return true;
          }
          else if (difftime(now + min_fresh, expire_time + max_stale) <= 0) {
            write_log(to_string(ID) + ": in cache, valid");
            write_log(to_string(ID) + ": NOTE=expired but accepted due to max-stale value");
            return true;
          }
          else if (difftime(now + min_fresh, expire_time + max_stale) > 0) { // can just be an else
            char date_time_string[100];
            strncpy(date_time_string, asctime(localtime(&cache_hit.exp_time)), strlen(asctime(localtime(&cache_hit.exp_time))) - 1);
            write_log(to_string(ID) + ": in cache, but expired at " + date_time_string);
          } 
        }
      }
    }
    else {  // should never trigger, error checking
      write_log(to_string(ID) + ": not in cache");
    }
  }
  else {
    write_log(to_string(ID) + ": not in cache");
  }
  return false;
}

pair<time_t, time_t> store_cache_information(int ID, string resp) {
  regex rg_cache("(Cache\\-Control\\: (.*))");
  smatch match;
  pair<time_t, time_t> times;
  time_t zero_time = 0;
  time_t request_time = time(NULL);
  // struct tm request_time = *gmtime(&now);
  times.first = request_time;

  if (regex_search(resp, match, rg_cache)) {
    // contains a Cache-Control tag
    if (match[2].str().find("no-store") != string::npos ||
        match[2].str().find("private") != string::npos) {
      write_log(to_string(ID) + ": not cacheable because " + "no-store/private flag");
      times.second = zero_time;
    }
    else if (match[2].str().find("no-cache") != string::npos) {
      write_log(to_string(ID) + ": cached, but requires re-validation");
      times.second = request_time;
    }
    else if (match[2].str().find("max-age") != string::npos) {
      regex rgx_exp("(max\\-age\\=([0-9]+))");
      smatch exp_match;
      string const temp = match[2].str();
      if (!regex_search(temp, exp_match, rgx_exp)) {
        write_log("ERROR malformatted max-age field in response");
        times.first = zero_time;
        times.second = zero_time;
        return times;
      }
      int max_age = stoi(exp_match[2].str());
      if (max_age == 0 && match[2].str().find("must-revalidate") != string::npos) {
        write_log(to_string(ID) + ": cached, but requires re-validation");
      }
      else {
        request_time += max_age;
        char date_time_string[100];
        strncpy(date_time_string, asctime(localtime(&request_time)), strlen(asctime(localtime(&request_time))) - 1);
        write_log(to_string(ID) + ": cached, expires at " + date_time_string);
      }
      times.second = request_time;
    }
    else if (match[2].str().find("must-revalidate") != string::npos) { // should never need but is just an error catching case?
      write_log(to_string(ID) + ": cached, expires at " + "No Expire Time Given");
      times.second = request_time;
    }
    else {
      write_log(to_string(ID) + ": not cacheable because " + "Cache-Control flag there, but had no cache information");
      times.second = zero_time;
    }
  }
  else {
    write_log(to_string(ID) + ": not cacheable because " + "server gave no cache information");
    times.second = zero_time;
  }
  return times;
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
  string const strHeaders = boost::lexical_cast<string>(forward_request.base());
  // cache return object
  pair<time_t, time_t> times;
  bool in_cache = false;

  if (client_request.method_string() == "GET") {
    regex rgx_cache("(Cache\\-Control\\: (.*))");
    regex age_rgx("(max\\-age\\=([0-9]+))");
    regex stale_rgx("(max\\-stale\\=([0-9]+))");
    regex fresh_rgx("(max\\-fresh\\=([0-9]+))");
    smatch match, age_match, stale_match, fresh_match;
    int max_age_client = -1;
    int max_stale = 0;
    int min_fresh = 0;
    if (regex_search(strHeaders, match, rgx_cache)) {
      if (match[2].str().find("max-age") != string::npos) {
        string const age_str = match[2].str();
        if (!regex_search(age_str, age_match, age_rgx)) {
          write_log("ERROR malformatted max-age field in request");
          http::response<http::string_body> response{http::status::bad_request, client_request.version()};
          http::write(client_socket, response);
          return;
        }
        max_age_client = stoi(age_match[2].str());
      }
      if (match[2].str().find("max-stale") != string::npos) {
        string const stale_str = match[2].str();
        if (!regex_search(stale_str, stale_match, stale_rgx)) {
          write_log("ERROR malformatted max-stale field in request");
          http::response<http::string_body> response{http::status::bad_request, client_request.version()};
          http::write(client_socket, response);
          return;
        }
        max_stale = stoi(stale_match[2].str());
      }
      if (match[2].str().find("min-fresh") != string::npos) {
        string const fresh_str = match[2].str();
        if (!regex_search(fresh_str, fresh_match, fresh_rgx)) {
          write_log("ERROR malformatted max-stale field in request");
          http::response<http::string_body> response{http::status::bad_request, client_request.version()};
          http::write(client_socket, response);
          return;
        }
        min_fresh = stoi(fresh_match[2].str());
      }
    }
    in_cache = check_cache(ID, string(client_request.target()), host, max_age_client, max_stale, min_fresh);
  }

  if (in_cache) {
    cache_entry cache_hit = cache.find(string(client_request.target()))->second;
    std::string const cached_response_headers =
        boost::lexical_cast<std::string>(cache_hit.res_body.base());
    http::write(client_socket, cache_hit.res_body);
    write_log(to_string(ID) + ": Responding \"" +
              cached_response_headers.substr(0, cached_response_headers.find("\n") - 1) +
              "\"");
    if (cached_response_headers.find("200 OK") != std::string::npos) {
      write_log(to_string(ID) + ": Tunnel closed");
    }
    return;
  }

  write_log(to_string(ID) + ": Requesting \"" + strHeaders.substr(0, strHeaders.find("\n")-1) + "\" from " + host);

  ip::tcp::socket server_socket = setUpSocketToConnect(host, port, ioc);

  //Setting up the request body to send to server
  forward_request.target(path);

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

  if (response_headers.find("200 OK") != std::string::npos) {
    if (client_request.method_string() == "GET") {
      times = store_cache_information(ID, response_headers);
      if (difftime(times.first, 0) == 0) {
        write_log("ERROR malformatted max-age field in response, not added to cache");
        return;
      }
      if (difftime(times.second, times.first) >= 0) {
        const std::lock_guard<std::mutex> lock(cache_mtx);
        cache_entry enter = cache_entry{};
        enter.res_time = times.first;
        enter.exp_time = times.second;
        enter.res_body = response;
        if (cache.find(string(client_request.target())) != cache.end()) {
          cache.at(string(client_request.target())) = enter;
        }
        else {
          cache.insert({string(client_request.target()), enter});
        }
      }
    }
    write_log(to_string(ID) + ": Tunnel closed");
  }
  return;
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
    write_log(to_string(ID) + ": WARNING " + error.message() + "Sending Bad Request");
    //Send a Response 400 Bad Request back to client
    http::response<http::string_body> response{http::status::bad_request,
                                               request.version()};
    http::write(socket, response);
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
    write_log(to_string(ID) +
              ": WARNING Unknown HTTP request made Responding 400 Bad request");
    //Send a Response 400 Bad Request back to client
    http::response<http::string_body> response{http::status::bad_request,
                                               request.version()};
    http::write(socket, response);
  }

  socket.shutdown(ip::tcp::socket::shutdown_both);
}

int main(int argc, char ** argv) {
  logfile.open("/var/log/erss/proxy.log");

  ip::address addr = ip::make_address("0.0.0.0");
  unsigned short port_num = 12345;

  io_context ioc{1};
  //Listen to new connection
  ip::tcp::acceptor acceptor{ioc, {addr, port_num}};
  int ID = 100; // care about starting at 1 or 100?

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
