#ifndef __DICT_COLLECTIONS_H__
#define __DICT_COLLECTIONS_H__

#include <iostream>
#include <vector>
#include "stringmap.hpp"

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
    void dump(ostream& out);

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
    StringMap<int>          database;
    DictionaryCollections * collections;

    int retrieve(const char * key, bool create) {
        int val = 0;
        if (database.contains(key)) {
            database.unsafe_get(key, val);
            return val;
        } else {
            if (create) {
                val = collections->idx;
                database.unsafe_set(key, val);
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
    for (int i = 0; i < dicts.size(); ++ i) {
        out << dicts[i]->dict_name << ":{";
        for (StringMap<int>::const_iterator itx = dicts[i]->database.begin();
                itx != dicts[i]->database.end();
                ++ itx) {
            out << itx->first << ":" << itx->second << ",";
        }
        out << "},";
    }
}

}       //  end for namespace parser
}       //  end for namespace ltp
#endif  //  end for __FEATURE_COLLECTIONS_H__
