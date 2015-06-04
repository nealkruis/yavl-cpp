#include <stdio.h>
#include <assert.h>
#include "yavl.h"

using namespace std;
using namespace YAVL;

namespace YAVL {
template<>
std::string ctype2str<unsigned long long>() {
	return "unsigned long long";
}

template<>
std::string ctype2str<string>() {
	return "string";
}

template<>
std::string ctype2str<long long>() {
	return "long long";
}

template<>
std::string ctype2str<unsigned int>() {
	return "unsigned int";
}

template<>
std::string ctype2str<int>() {
	return "int";
}

}

ostream& operator <<(ostream& os, const Path& path) {
	for (Path::const_iterator i = path.begin(); i != path.end(); ++i) {
		// no dot before list indexes and before first element
		if ((i != path.begin()) && ((*i)[0] != '[')) {
			os << '.';
		}
		os << *i;
	}
	return os;
}

ostream& operator <<(ostream& os, const Exception& v) {
	os << "REASON: " << v.why << endl;
	os << "  doc path: " << v.doc_path << endl;
	os << "  treespec path: " << v.gr_path << endl;
	os << endl;
	return os;
}

ostream& operator <<(ostream& os, const Errors& v) {
	for (Errors::const_iterator i = v.begin(); i != v.end(); ++i) {
		os << *i;
	}
	return os;
}

const string& Validator::type2str(const YAML::Node &doc) {
	static string nonestr = "none";
	static string scalarstr = "scalar";
	static string liststr = "list";
	static string mapstr = "map";

	if (doc.IsNull()) {
		return nonestr;
	} else if (doc.IsScalar()) {
		return scalarstr;
	} else if (doc.IsSequence()) {
		return liststr;
	} else if (doc.IsMap()) {
		return mapstr;
	} else {
		assert(0);
		return nonestr;
	}
	assert(0);
	return nonestr;
}

int Validator::num_keys(const YAML::Node& doc) {
	if (!doc.IsMap()) {
		return 0;
	}
	int num = 0;
	for (YAML::const_iterator i = doc.begin(); i != doc.end(); ++i) {
		num++;
	}
	return num;
}

bool Validator::validate_map(const YAML::Node &mapNode, const YAML::Node &doc) {
	if (!doc.IsMap()) {
		string reason = "expected map, but found " + type2str(doc);
		gen_error(Exception(reason, gr_path, doc_path));
		return false;
	}

	bool ok = true;
	for (YAML::const_iterator i = mapNode.begin(); i != mapNode.end(); ++i) {
		const YAML::Node& first = i->first;
		string key = first.as<string>();

		const YAML::Node& attributeNode = i->second; // document node to validate in next recursion
		const YAML::Node docMapNode = doc[key];

		if (!docMapNode.IsDefined()) {
			string reason = "key: " + key + " not found.";
			gen_error(Exception(reason, gr_path, doc_path));
			ok = false;
		} else {
			doc_path.push_back(key);
			gr_path.push_back(key);

			ok = validate_doc(attributeNode, docMapNode) && ok;

			doc_path.pop_back();
			gr_path.pop_back();
		}
	}
	return ok;
}

bool Validator::validate_leaf(const YAML::Node &gr, const YAML::Node &doc) {
	assert(gr.IsSequence());

	const YAML::Node& typespec_map = gr[0];
	assert(num_keys(typespec_map) == 1);

	string type = typespec_map.begin()->first.as<string>();
	const YAML::Node& type_specifics = typespec_map.begin()->second;

	bool ok = true;
	if (type == "string") {
		string string_leaf = doc.as<string>();
		if (string_leaf.empty())
			ok = false;
	} else {
		try {
			if (type == "uint64") {
				doc.as<unsigned long long>();
			} else if (type == "int64") {
				doc.as<long long>();
			} else if (type == "int") {
				doc.as<int>();
			} else if (type == "uint") {
				doc.as<unsigned int>();
			}
			else if (type == "double") {
				doc.as<double>();
			}
		} catch (YAML::TypedBadConversion<unsigned long long> &tbc_e) {
			// do something with tbc_e
			string reason = "Expected unsigned long long leaf attribute value: "
					+ tbc_e.msg;
			gen_error(Exception(reason, gr_path, doc_path));
			ok = false;
		} catch (YAML::TypedBadConversion<long long> &tbc_e) {
			// do something with tbc_e
			string reason = "Expected long long leaf attribute value: "
					+ tbc_e.msg;
			gen_error(Exception(reason, gr_path, doc_path));
			ok = false;
		} catch (YAML::TypedBadConversion<int> &tbc_e) {
			// do something with tbc_e
			string reason = "Expected int leaf attribute value: " + tbc_e.msg;
			gen_error(Exception(reason, gr_path, doc_path));
			ok = false;
		} catch (YAML::TypedBadConversion<unsigned int> &tbc_e) {
			// do something with tbc_e
			string reason = "Expected unsigned unsigned int leaf attribute value: "
					+ tbc_e.msg;
			gen_error(Exception(reason, gr_path, doc_path));
			ok = false;
		} catch (YAML::TypedBadConversion<double> &tbc_e) {
			// do something with tbc_e
			string reason = "Expected unsigned double leaf attribute value: "
					+ tbc_e.msg;
			gen_error(Exception(reason, gr_path, doc_path));
			ok = false;
		}
	}
	if (type == "enum") {
		ok = false;
		string docValue = doc.as<string>();
		for (YAML::const_iterator i = type_specifics.begin();
				i != type_specifics.end(); ++i) {
			string enumvalue_leaf = i->as<string>();
			if (enumvalue_leaf == docValue) {
				ok = true;
				break;
			}
		}
		if (!ok) {
			string reason = "enum string '" + docValue + "' is not allowed.";
			gen_error(Exception(reason, gr_path, doc_path));
		}
	}
	return ok;
}

bool Validator::validate_list(const YAML::Node &gr, const YAML::Node &doc) {
	if (!doc.IsSequence()) {
		string reason = "expected list, but found " + type2str(doc);
		gen_error(Exception(reason, gr_path, doc_path));
		return false;
	}

	bool ok = true;
	int n = 0;
	char buf[128];

	for (YAML::const_iterator i = doc.begin(); i != doc.end(); ++i, ++n) {
		snprintf(buf, sizeof(buf), "[%d]", n);
		doc_path.push_back(buf);

		ok = validate_doc(gr, *i) && ok; // next recursion step for validating a document node
		doc_path.pop_back();
	}
	return ok;
}

bool Validator::validate_doc(const YAML::Node &gr, const YAML::Node &doc) {
	bool ok = true;

	const YAML::Node mapNode = gr["map"];
	const YAML::Node listNode = gr["list"];

	if (mapNode.IsDefined()) {
		gr_path.push_back("map");
		ok = validate_map(mapNode, doc) && ok;
		gr_path.pop_back();
	} else if (listNode.IsDefined()) {
		gr_path.push_back("list");
		ok = validate_list(listNode, doc) && ok;
		gr_path.pop_back();
	} else {
		ok = validate_leaf(gr, doc) && ok;
	}
	return ok;
}
