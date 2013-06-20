#ifndef __DICT_COLLECTIONS_H__
#define __DICT_COLLECTIONS_H__

#include <iostream>
#include <vector>
#include "stringmap.hpp"
#include "smartmap.hpp"

namespace ltp {
namespace parser {

using namespace std;
using namespace ltp::utility;

class Dictionary;

class DictionaryCollections {
public:
    DictionaryCollections() : idx(0) {}
    ~DictionaryCollections() {}

    Dictionary * create_dict(const char * name);
    void dump(ostream & out);
    bool load(istream & in);

    size_t dim() const {
        return idx;
    }
public:
    int idx;

private:
    vector<Dictionary *> dicts;
};

class Dictionary {
public:
    Dictionary(const char * name,
            DictionaryCollections * coll): 
        dict_name(name), 
        collections(coll) {}

    string                  dict_name;
    //StringMap<int>          database;
    SmartMap<int>           database;
    DictionaryCollections * collections;

    int retrieve(const char * key, bool create) {
        int val = 0;
        if (database.contains(key)) {
            // database.unsafe_get(key, val);
            database.get(key, val);
            return val;
        } else {
            if (create) {
                val = collections->idx;
                database.set(key, val);
                // database.unsafe_set(key, val);
                collections->idx ++;
                return val;
            } else {
                return -1;
            }
        }

        return -1;
    }
};

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
    out.write(reinterpret_cast<const char *>(&sz), sizeof(unsigned int));
    for (int i = 0; i < dicts.size(); ++ i) {
        strncpy(chunk, dicts[i]->dict_name.c_str(), 32);
        out.write(chunk, 32);

        dicts[i]->dump(out);
    }
}

bool DictionaryCollections::load(istream & in) {
    char chunk[32];
    unsigned int sz;

    in.read(chunk, 16);
    if (strcmp(chunk, "collections")) {
        return false;
    }

    in.read(reinterpret_cast<char *>(&sz), sizeof(unsigned int));
    for (int i = 0; i < sz; ++ i) {
        in.read(chunk, 32);

        Dictionary * dict = create_dict(chunk);
        if (!dict->load(in)) {
            return false;
        }

        dicts.push_back(dict);
    }

    return true;
}

}       //  end for namespace parser
}       //  end for namespace ltp
#endif  //  end for __FEATURE_COLLECTIONS_H__
