#include "collections.h"

namespace ltp {
namespace parser {

size_t DictionaryCollections::dim() const{
    return idx;
}

Dictionary * DictionaryCollections::create_dict(const char * name) {
    for (int i = 0; i < dicts.size(); ++ i) {
        if ( strcmp(dicts[i]->dict_name.c_str(), name) == 0) {
            return dicts[i];
        }
    }
    Dictionary * dict = new Dictionary(name, this);
    dicts.push_back(dict);
    return dict;
}

void DictionaryCollections::dump(ostream & out) {
    char chunk[32];
    unsigned int sz = dicts.size();
    strncpy(chunk, "collections", 16);

    out.write(chunk, 16);
    out.write(reinterpret_cast<const char *>(&idx), sizeof(int));
    out.write(reinterpret_cast<const char *>(&sz), sizeof(unsigned int));
    for (int i = 0; i < dicts.size(); ++ i) {
        strncpy(chunk, dicts[i]->dict_name.c_str(), 32);
        out.write(chunk, 32);

        dicts[i]->database.dump(out);
    }
}

bool DictionaryCollections::load(istream & in) {
    char chunk[32];
    unsigned int sz;

    in.read(chunk, 16);
    if (strcmp(chunk, "collections")) {
        return false;
    }

    in.read(reinterpret_cast<char *>(&idx), sizeof(int));
    in.read(reinterpret_cast<char *>(&sz), sizeof(unsigned int));
    for (unsigned i = 0; i < sz; ++ i) {
        in.read(chunk, 32);

        Dictionary * dict = create_dict(chunk);
        if (!dict->database.load(in)) {
            return false;
        }

        dicts.push_back(dict);
    }

    return true;
}


}   //  end for namespace parser
}   //  end for namespace ltp
