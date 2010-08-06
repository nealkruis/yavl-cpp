#include <vector>
#include <string>
#include "yaml.h"
#include "yaml_dumper.h"
struct A {
  std::string a1;
  std::string a2;
};
void operator >>(const YAML::Node& node, A &obj);
YAML::Emitter& operator <<(YAML::Emitter& out, const A &obj);
struct Pieces {
  std::vector<A > a;
  std::vector<int > b;
};
void operator >>(const YAML::Node& node, Pieces &obj);
YAML::Emitter& operator <<(YAML::Emitter& out, const Pieces &obj);
enum Size { big, small };
void operator >>(const YAML::Node& node, Size &obj);
YAML::Emitter& operator <<(YAML::Emitter& out, const Size &obj);
struct Header {
  std::string name;
  Pieces pieces;
  Size size;
  std::string version;
};
void operator >>(const YAML::Node& node, Header &obj);
YAML::Emitter& operator <<(YAML::Emitter& out, const Header &obj);
struct Top {
  Header header;
};
void operator >>(const YAML::Node& node, Top &obj);
YAML::Emitter& operator <<(YAML::Emitter& out, const Top &obj);
