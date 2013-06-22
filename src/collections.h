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

    inline int size() {
        return database.size();
    }
};

class LabelCollections : public SmartMap<int> {
public:
    LabelCollections() : entries(0), cap_entries(0) {}

    ~LabelCollections() {
        if (entries) {
            delete [](entries);
        }
    }

private:
    int cap_entries;
    int * entries;

public:
    int push(const char * key) {
        if (!contains(key)) {
            int idx = SmartMap<int>::size();
            set(key, idx);

            if (cap_entries < SmartMap<int>::_num_entries) {
                cap_entries = ( SmartMap<int>::_num_entries << 1);
                int * new_entries = new int[cap_entries];
                if ( entries ) {
                    memcpy(new_entries, entries, sizeof(int) * (SmartMap<int>::_num_entries - 1));
                    delete [](entries);
                }
                entries = new_entries;
            }

            // SmartMap<int>::debug(cout);
            entries[_num_entries-1] = SmartMap<int>::_latest_hash_node->__key_off;
            return idx;
        } else {
            return (*get(key));
        }
    }

    const char * at(int i) {
        if (i >= 0 && i < _num_entries) {
            return SmartMap<int>::_key_buffer + entries[i];
        } else {
            return 0;
        }
    }

    int index( const char * key ) {
        int val = -1;
        if (get(key, val)) {
            return val;
        }

        return -1;
    }

    void dump(ostream & out) {
        SmartMap<int>::dump(out);
        out.write(reinterpret_cast<const char *>(entries), sizeof(int) * _num_entries);
    }

    bool load(istream & in) {
        SmartMap<int>::load(in);

        if (entries) {
            delete [](entries);
        }

        entries = new int[SmartMap<int>::_num_entries];
        in.read(reinterpret_cast<char *>(entries), sizeof(int) * _num_entries);
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

}       //  end for namespace parser
}       //  end for namespace ltp
#endif  //  end for __FEATURE_COLLECTIONS_H__
