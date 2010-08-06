#include "top.h"
using namespace std;
void operator >>(const YAML::Node& node, A &obj) {
  node["a1"] >> obj.a1;
  node["a2"] >> obj.a2;
}
void operator >>(const YAML::Node& node, Pieces &obj) {
  node["a"] >> obj.a;
  node["b"] >> obj.b;
}
void operator >>(const YAML::Node& node, Size &obj) {
  string tmp; node >> tmp;
  if (tmp == "big") obj = big;
  if (tmp == "small") obj = small;
}
void operator >>(const YAML::Node& node, Header &obj) {
  node["name"] >> obj.name;
  node["pieces"] >> obj.pieces;
  node["size"] >> obj.size;
  node["version"] >> obj.version;
}
void operator >>(const YAML::Node& node, Top &obj) {
  node["header"] >> obj.header;
}
YAML::Emitter& operator <<(YAML::Emitter& out, const A &obj) {
  out << YAML::BeginMap;
  out << YAML::Key << "a1";
  out << YAML::Value << obj.a1;
  out << YAML::Key << "a2";
  out << YAML::Value << obj.a2;
  out << YAML::EndMap;
  return out;
}
YAML::Emitter& operator <<(YAML::Emitter& out, const Pieces &obj) {
  out << YAML::BeginMap;
  out << YAML::Key << "a";
  out << YAML::Value << obj.a;
  out << YAML::Key << "b";
  out << YAML::Value << obj.b;
  out << YAML::EndMap;
  return out;
}
YAML::Emitter& operator <<(YAML::Emitter& out, const Size &obj) {
  if (obj == big) out << "big";
  if (obj == small) out << "small";
  return out;
}
YAML::Emitter& operator <<(YAML::Emitter& out, const Header &obj) {
  out << YAML::BeginMap;
  out << YAML::Key << "name";
  out << YAML::Value << obj.name;
  out << YAML::Key << "pieces";
  out << YAML::Value << obj.pieces;
  out << YAML::Key << "size";
  out << YAML::Value << obj.size;
  out << YAML::Key << "version";
  out << YAML::Value << obj.version;
  out << YAML::EndMap;
  return out;
}
YAML::Emitter& operator <<(YAML::Emitter& out, const Top &obj) {
  out << YAML::BeginMap;
  out << YAML::Key << "header";
  out << YAML::Value << obj.header;
  out << YAML::EndMap;
  return out;
}
