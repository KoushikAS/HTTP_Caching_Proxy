/**
Citations:
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
*/

#include<iostream>
#include<cstdlib>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>


void do_session(boost::asio::ip::tcp::socket& socket){
  boost::beast::flat_buffer buff;
  boost::beast::error_code error_code;

  boost::beast::http::request<boost::beast::http::string_body> request;
  
  boost::beast::http::read(socket, buff, request, error_code);
  //Checking for error 
  if(error_code && error_code != boost::beast::http::error::end_of_stream){
    std::cerr<<"Error "<<error_code.message()<<std::endl;
  }

  if(request.method() == boost::beast::http::verb::get){
    std::cout<<"Received a GET method"<<std::endl;
    std::cout<<request<<std::endl;
  }
  else{
    std::cout<<"Not a get method"<<std::endl;
  }
  
  socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, error_code);
}


int main(int argc, char **argv){

  boost::asio::ip::address addr = boost::asio::ip::make_address("127.0.0.1");
  unsigned short port_num = 12345;

  boost::asio::io_context ioc{1};

  boost::asio::ip::tcp::acceptor acceptor{ioc, {addr, port_num}};

  //Will Receive new connection
  boost::asio::ip::tcp::socket socket{ioc};

  std::cout<<"Waiting for connection at "<<std::endl;
  //Wait for the connection
  acceptor.accept(socket);

  do_session(socket);

  std::cout<<"Ending the server"<<std::endl;
  return EXIT_SUCCESS;
}