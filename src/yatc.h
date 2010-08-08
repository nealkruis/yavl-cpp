#ifndef _YATC_H_
#define _YATC_H_

#include "yaml.h"
#include <vector>
#include <string>
#include <ostream>
#include <utility>

namespace YAVL
{
  std::string to_lower_copy(std::string s);

  // intentionally named 'kind' to avoid confusion with 'type' :)
  enum KindOfDataNodeDefinition { BUILTIN, VECTOR, STRUCT, ENUM };

  struct EnumDefinition {
    std::string name;
    std::vector<std::string> enum_values;
  };

  struct DataNodeDefinition {
    KindOfDataNodeDefinition kind_of_node; // what kind of definition is this
    std::string name; // identifier to which data in this node will be bound
    std::string type; // C++-compatible type
    EnumDefinition enum_def; // if node is for an ENUM
    DataNodeDefinition* listelem_def;  // type of list elements if node is for VECTOR
    std::vector<DataNodeDefinition*> elems; // types of members if node is for STRUCT
  };

  class DataBinderGen {
  protected:
    const YAML::Node& gr;  // tree for grammar
    DataNodeDefinition root_data_defn;
    std::string topname; // name of top-level data struct.

    DataNodeDefinition make_types(const YAML::Node& doc, std::string name);
    DataNodeDefinition make_list_type(const YAML::Node &gr, std::string name);
    DataNodeDefinition make_map_type(const YAML::Node &mapNode, std::string name);
    DataNodeDefinition make_scalar_type(const YAML::Node &gr, std::string name);

    void emit_enum_def(const DataNodeDefinition &elem, std::ostream& os);
    bool emit_header(const DataNodeDefinition &elem, std::ostream& os);
    void emit_enum_reader(const DataNodeDefinition &elem, std::ostream& os);
    bool emit_reader(const DataNodeDefinition &elem, std::ostream& os);

    virtual void emit_enum_dumper(const DataNodeDefinition &elem, std::ostream& os);
    virtual bool emit_dumper(const DataNodeDefinition &elem, std::ostream& os);

    bool write_put_operator_epilog(std::ostream& os);
    bool write_put_operator_prolog(std::ostream& os,
      std::string type, bool prototype = false);
  public:
    DataBinderGen(const YAML::Node& _gr, std::string _topname) :
      gr(_gr), topname(_topname) {
        root_data_defn = make_types(gr, topname);
    };
    bool emit_header(std::ostream& os);
    bool emit_reader(std::ostream& os);
    virtual bool emit_dumper(std::ostream& os);
  };

}

#endif
