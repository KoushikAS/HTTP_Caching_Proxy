#include <iostream>
#include <cstdlib>
#include <fstream>
using namespace std;

int main(int argc, char * argv){
  std::cout << "Hello" << std::flush;
  ofstream myfile;
  std::cout.flush();
  myfile.open ("/var/log/erss/proxy.log");
  myfile << "Writing this to a file.\n";
  myfile.close();
  return EXIT_SUCCESS;
}