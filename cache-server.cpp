#include <iostream>
#include <cstdlib>
#include <fstream>
#include <stdio.h>
using namespace std;

int main(int argc, char * argv) {
  int count = 0;
  ofstream myfile;
  FILE * pFile;
  ifstream f("/var/log/erss/proxy.log");

  while (1) {
    if (count % 1000 == 0) {
      printf("count = %d\n", count);
      fprintf(pFile, "This is the %d cycle\n", count);
    }
    // fprintf(stdout, "This is the %d cycle\n", count);
    // cout << "hello" << endl;
    count++;
  }

  return EXIT_SUCCESS;
}