#include <fstream>
#include <iostream>
#include "top.h"

using namespace std;

int main(int argc, char **argv)
{
  std::ifstream fin;
  fin.open(argv[1]);
  
  Top top;

  try {
    YAML::Parser parser(fin);
    YAML::Node doc;
    parser.GetNextDocument(doc);

    // read YAML file into our data structures
    doc >> top;

    // write out our data structure as a YAML
    YAML::Emitter out;
    // first build the in-memory YAML tree
    out << top;

    // dump it to disk
    cout << out.c_str() << endl;
  } catch(const YAML::Exception& e) {
    std::cerr << e.what() << "\n";
  }
  return 0;
}
