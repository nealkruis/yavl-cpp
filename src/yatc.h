#ifndef _YATC_H_
#define _YATC_H_

#include "yaml.h"
#include <vector>
#include <string>
#include <ostream>
#include <utility>

namespace YAVL
{
  enum ElemType { SCALAR, VECTOR, STRUCT, ENUM };

  struct EnumDef {
    std::string name;
    std::vector<std::string> enum_values;
  };

  struct StructElem {
    ElemType elem_type;
    std::string name;
    std::string type;
    EnumDef enum_def;
    StructElem* vector_type;  // type of list elements. ignored for others.
    std::vector<StructElem*> elems; // types of members. ignored except in struct.
  };

  typedef std::pair<StructElem, bool> RslType;

  class RWGen {
  protected:
    const YAML::Node& gr;
    RslType types;
    std::string topname;
    std::string dumper_header;

    RslType make_types(const YAML::Node& doc, std::string name);
    RslType make_list_type(const YAML::Node &gr, std::string name);
    RslType make_map_type(const YAML::Node &mapNode, std::string name);
    RslType make_scalar_type(const YAML::Node &gr, std::string name);

    void emit_enum_def(const StructElem &elem, std::ostream& os);
    bool emit_header(const StructElem &elem, std::ostream& os);
    void emit_enum_reader(const StructElem &elem, std::ostream& os);
    bool emit_reader(const StructElem &elem, std::ostream& os);

    virtual void emit_enum_dumper(const StructElem &elem, std::ostream& os);
    virtual bool emit_dumper(const StructElem &elem, std::ostream& os);

    bool write_put_operator_epilog(std::ostream& os);
    bool write_put_operator_prolog(std::ostream& os,
      std::string type, bool prototype = false);
  public:
    RWGen(const YAML::Node& _gr, std::string _topname) :
      gr(_gr), topname(_topname), dumper_header("yaml_dumper.h") {
        types = make_types(gr, topname);
    };
    bool emit_header(std::ostream& os);
    bool emit_reader(std::ostream& os);
    virtual bool emit_dumper(std::ostream& os);
  };

#if 0
  class RWYAMLGen : protected RWGen {
  public:
    RWYAMLGen(const YAML::Node& _gr, std::string _topname) :
      RWGen(_gr, _topname) {
        dumper_header = std::string("yaml_dumper.h");
      };
      
    virtual bool emit_dumper(std::ostream& os);
    virtual void emit_enum_dumper(const StructElem &elem, std::ostream& os);
    virtual bool emit_dumper(const StructElem &elem, std::ostream& os);
  };
#endif
}

#endif
