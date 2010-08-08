#include <fstream>
#include <iostream>
#include <string>
#include "yaml.h"
#include "yatc.h"

using namespace std;

int main(int argc, char **argv)
{
  std::ifstream grin; // grammar file
  grin.open(argv[1]);
  
  YAML::Node gr;
  try {
    YAML::Parser parser(grin);
    parser.GetNextDocument(gr);
  } catch(const YAML::Exception& e) {
    std::cerr << "Error reading grammar: " << e.what() << "\n";
    return 1;
  }

  string topname(argv[2]);
  YAVL::DataBinderGen yatc(gr, topname);

  ofstream hf;
  hf.open((topname + ".h").c_str());
  yatc.emit_header(hf);
  hf.close();

  ofstream rf;
  rf.open((topname + ".cpp").c_str());
  yatc.emit_reader(rf);
  yatc.emit_dumper(rf);
  rf.close();
}


