#include <stdio.h>
#include <assert.h>
#include "yaml.h"
#include "yatc.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <ctype.h>

using namespace std;
using namespace YAVL;
using namespace boost;

RslType RWGen::make_map_type(const YAML::Node &mapNode, string name)
{
  bool ok = true;
  StructElem rsl;
  rsl.elem_type = STRUCT;
  rsl.type = to_lower_copy(name);
  rsl.type[0] = toupper(rsl.type[0]);
  rsl.name = name;

  for (YAML::Iterator i = mapNode.begin(); i != mapNode.end(); ++i) {
    string key = i.first();
    const YAML::Node &valueNode = i.second();
    RslType subrsl = make_types(valueNode, key);
    StructElem *e = new StructElem(subrsl.first);
    rsl.elems.push_back(e);
    ok = subrsl.second && ok;
  }
  return make_pair(rsl, ok);
}

RslType RWGen::make_scalar_type(const YAML::Node &doc, string name)
{
  assert( doc.GetType() == YAML::CT_SEQUENCE );

  const YAML::Node& typespec_map = doc[0];
  //assert( num_keys(typespec_map) == 1);

  string type = typespec_map.begin().first();
  const YAML::Node& type_specifics = typespec_map.begin().second();

  StructElem elem;

  elem.name = name;

  bool ok = true;
  if (type == "string") {
    elem.type = "std::string";
    elem.elem_type = SCALAR;
  } else if (type == "uint64") {
    elem.type = "unsigned long long";
    elem.elem_type = SCALAR;
  } else if (type == "int64") {
    elem.type = "long long";
    elem.elem_type = SCALAR;
  } else if (type == "int") {
    elem.type = "int";
    elem.elem_type = SCALAR;
  } else if (type == "uint") {
    elem.type = "unsigned int";
    elem.elem_type = SCALAR;
  } else if (type == "enum") {
    elem.enum_def.name = to_lower_copy(elem.name);
    elem.enum_def.name[0] = toupper(elem.enum_def.name[0]);
    for (YAML::Iterator i = type_specifics.begin(); i != type_specifics.end(); ++i) {
      elem.enum_def.enum_values.push_back(*i);
    }
    elem.elem_type = ENUM;
    elem.type = elem.enum_def.name;
  } else {
    ok = false;
  }
  return make_pair(elem, ok);
}

RslType RWGen::make_list_type(const YAML::Node &doc, string name)
{
  StructElem elem;
  elem.name = name;
  elem.elem_type = VECTOR;
  
  //assert(num_keys(doc) == 1);

  string vec_elem_type = to_lower_copy(name);
  vec_elem_type[0] = toupper(vec_elem_type[0]);
  if (*(vec_elem_type.end()-1) == 's') {
     vec_elem_type.erase(vec_elem_type.end()-1);
  }
  RslType subrsl = make_types(doc, vec_elem_type);
  StructElem *e = new StructElem(subrsl.first);
  elem.vector_type = e;

  elem.type = string("std::vector<") + elem.vector_type->type + string(" >");

  return make_pair(elem, true);
}

RslType RWGen::make_types(const YAML::Node& doc, string name)
{
  const YAML::Node *mapNode = 0;
  const YAML::Node *listNode = 0;
  RslType rsl;
  if ((mapNode = doc.FindValue("map"))) {
    rsl = make_map_type(*mapNode, name);
  } else if ((listNode = doc.FindValue("list"))) {
    rsl = make_list_type(*listNode, name);
  } else {
    rsl = make_scalar_type(doc, name);
  }
  return rsl;
}

void RWGen::emit_enum_def(const StructElem &elem, ostream& os)
{
  os << "enum " << elem.enum_def.name << " { ";
  vector<string>::const_iterator i = elem.enum_def.enum_values.begin();
  for (; i != elem.enum_def.enum_values.end(); ++i) {
    if (i !=  elem.enum_def.enum_values.begin()) {
      os << ", ";
    }
    os << *i;
  }
  os << " };" << endl;
  os << "void operator >>(const YAML::Node& node, " << elem.type << " &obj);" << endl;
  write_put_operator_prolog(os, elem.type, true /* prototype */);
}

bool RWGen::emit_header(const StructElem &elem, ostream& os)
{
  switch (elem.elem_type) {
    case ENUM:
      emit_enum_def(elem, os);
      break;

    case VECTOR:
       if (elem.vector_type->elem_type != SCALAR) {
         emit_header(*elem.vector_type, os);
       }
      break;

    case SCALAR:
      break;

    case STRUCT:
      vector<StructElem*>::const_iterator i = elem.elems.begin();
      // pass one: emit what I depend on
      for (; i != elem.elems.end(); ++i) {
        if (((*i)->elem_type == ENUM)) {
          emit_enum_def(**i, os);
        } else if ((*i)->elem_type == STRUCT) {
          emit_header(**i, os);
        } else if ((*i)->elem_type == VECTOR) {
          if ((*i)->vector_type->elem_type != SCALAR) {
            emit_header(*((*i)->vector_type), os);
          }
        }
      }
      // pass two: emit myself
      os << "struct " << elem.type << " {" << endl;
      i = elem.elems.begin();
      for (; i != elem.elems.end(); ++i) {
        const StructElem& e = **i;
        os << "  " << e.type << " " << e.name << ";" << endl;
      }
      os << "};" << endl;
      os << "void operator >>(const YAML::Node& node, " << elem.type << " &obj);" << endl;
      write_put_operator_prolog(os, elem.type, true /* prototype */);
      break;
  }
  return true;
}

bool RWGen::emit_header(ostream& os)
{
  if (!types.second) {
    return false;
  }
  os << "#include <vector>" << endl;
  os << "#include <string>" << endl;
  os << "#include \"yaml.h\"" << endl;
  os << "#include \"" << dumper_header << "\"" << endl;
  emit_header(types.first, os);
  return true;
}

void RWGen::emit_enum_reader(const StructElem &elem, ostream& os)
{
  os << "void operator >>(const YAML::Node& node, " << elem.type << " &obj) {" << endl;
  os << "  string tmp; node >> tmp;" << endl;
  vector<string>::const_iterator i = elem.enum_def.enum_values.begin();
  for (; i != elem.enum_def.enum_values.end(); ++i) {
    os << "  if (tmp == \"" << *i << "\") obj = " << *i << ';' << endl;
  }
  os << "}" << endl;
}

bool RWGen::emit_reader(const StructElem &elem, ostream& os)
{
  switch (elem.elem_type) {
    case ENUM:
      emit_enum_reader(elem, os);
      os << "void operator >>(const YAML::Node& node, " << elem.type << " &obj) {" << endl;
      os << "  node[\"" << elem.name << "\"] >> obj." << elem.name << ";" << endl;
      os << "}" << endl;
      break;

    case VECTOR:
       if (elem.vector_type->elem_type != SCALAR) {
         emit_reader(*elem.vector_type, os);
       }
      break;

    case SCALAR:
      os << "void operator >>(const YAML::Node& node, " << elem.type << " &obj) {" << endl;
      os << "  node[\"" << elem.name << "\"] >> obj." << elem.name << ";" << endl;
      os << "}" << endl;
      break;

    case STRUCT:
      vector<StructElem*>::const_iterator i = elem.elems.begin();
      // pass one: emit what I depend on
      for (; i != elem.elems.end(); ++i) {
        if (((*i)->elem_type == ENUM)) {
          emit_enum_reader(**i, os);
        } else if ((*i)->elem_type == STRUCT) {
          emit_reader(**i, os);
        } else if ((*i)->elem_type == VECTOR) {
          if ((*i)->vector_type->elem_type != SCALAR) {
            emit_reader(*((*i)->vector_type), os);
          }
        }
      }
      // pass two: emit myself
      os << "void operator >>(const YAML::Node& node, " << elem.type << " &obj) {" << endl;
      i = elem.elems.begin();
      for (; i != elem.elems.end(); ++i) {
        const StructElem& e = **i;
        os << "  node[\"" << e.name << "\"] >> obj." << e.name << ";" << endl;
      }
      os << "}" << endl;
      break;
  }
  return true;
}

bool RWGen::emit_reader(ostream& os)
{
  if (!types.second) {
    return false;
  }
  os << "#include \"" << to_lower_copy(topname) << ".h\"" << endl;
  os << "using namespace std;" << endl;
  emit_reader(types.first, os);
  return true;
}

void RWGen::emit_enum_dumper(const StructElem &elem, ostream& os)
{
  write_put_operator_prolog(os, elem.type);
  vector<string>::const_iterator i = elem.enum_def.enum_values.begin();
  for (; i != elem.enum_def.enum_values.end(); ++i) {
    os << "  if (obj == " << *i << ") out << \"" << *i << "\";" << endl;
  }
  write_put_operator_epilog(os);
}

bool RWGen::write_put_operator_prolog(ostream& os, string type, bool prototype)
{
  os << "YAML::Emitter& operator <<(YAML::Emitter& out, const " << type << " &obj)";
  if (!prototype) {
    os << " {";
  } else {
    os << ";";
  }
  os << endl;
}

bool RWGen::write_put_operator_epilog(ostream& os)
{
  os << "  return out;" << endl;
  os << "}" << endl;
}

#if 0
bool RWGen::emit_dumper(const StructElem &elem, ostream& os)
{
  switch (elem.elem_type) {
    case ENUM:
      emit_enum_dumper(elem, os);
      write_put_operator_prolog(os, elem.type);
      os << "  os << \"" << elem.name << ": \" << obj." << elem.name << " << endl;" << endl;
      write_put_operator_epilog(os);
      break;

    case VECTOR:
       if (elem.vector_type->elem_type != SCALAR) {
         emit_dumper(*elem.vector_type, os);
       }
      break;

    case SCALAR:
      break;

    case STRUCT:
      vector<StructElem*>::const_iterator i = elem.elems.begin();
      // pass one: emit what I depend on
      for (; i != elem.elems.end(); ++i) {
        if (((*i)->elem_type == ENUM)) {
          emit_enum_dumper(**i, os);
        } else if ((*i)->elem_type == STRUCT) {
          emit_dumper(**i, os);
        } else if ((*i)->elem_type == VECTOR) {
          if ((*i)->vector_type->elem_type != SCALAR) {
            emit_dumper(*((*i)->vector_type), os);
          }
        }
      }
      // pass two: emit myself
      write_put_operator_prolog(os, elem.type);
      i = elem.elems.begin();
      for (; i != elem.elems.end(); ++i) {
        const StructElem& e = **i;
        os << "  os << \"" << e.name << ": \" << obj." << e.name << " << endl;" << endl;
      }
      write_put_operator_epilog(os);
      break;
  }
  return true;
}
#endif

bool RWGen::emit_dumper(const StructElem &elem, ostream& os)
{
  switch (elem.elem_type) {
    case ENUM:
      emit_enum_dumper(elem, os);
      write_put_operator_prolog(os, elem.type);
      os << "  out << obj." << elem.name << ";" << endl;
      write_put_operator_epilog(os);
      break;

    case VECTOR:
       if (elem.vector_type->elem_type != SCALAR) {
         emit_dumper(*elem.vector_type, os);
       }
      break;

    case SCALAR:
      break;

    case STRUCT:
      vector<StructElem*>::const_iterator i = elem.elems.begin();
      // pass one: emit what I depend on
      for (; i != elem.elems.end(); ++i) {
        if (((*i)->elem_type == ENUM)) {
          emit_enum_dumper(**i, os);
        } else if ((*i)->elem_type == STRUCT) {
          emit_dumper(**i, os);
        } else if ((*i)->elem_type == VECTOR) {
          if ((*i)->vector_type->elem_type != SCALAR) {
            emit_dumper(*((*i)->vector_type), os);
          }
        }
      }
      // pass two: emit myself
      write_put_operator_prolog(os, elem.type);
      os << "  out << YAML::BeginMap;" << endl;
      i = elem.elems.begin();
      for (; i != elem.elems.end(); ++i) {
        const StructElem& e = **i;
        os << "  out << YAML::Key << \"" << e.name << "\";" << endl;
        os << "  out << YAML::Value << obj." << e.name << ";" << endl;
      }
      os << "  out << YAML::EndMap;" << endl;
      write_put_operator_epilog(os);
      break;
  }
  return true;
}
bool RWGen::emit_dumper(ostream& os)
{
  if (!types.second) {
    return false;
  }
  emit_dumper(types.first, os);
  return true;
}

