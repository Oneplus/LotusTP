#ifndef __DICT_COLLECTIONS_H__
#define __DICT_COLLECTIONS_H__

#include <iostream>
#include <vector>
#include "stringmap.hpp"
#include "smartmap.hpp"

#include "instance.h"

namespace ltp {
namespace parser {

using namespace std;
using namespace ltp::utility;


// declariation of dictionary, this is specially needed
// by the observer design pattern
class Dictionary;

// class of a collection of dictionary
// a index counter is shared within several dictionary.
class DictionaryCollections {
public:
    DictionaryCollections(int num_dicts);
    ~DictionaryCollections() {}

    /*
     * Dump the dictionary collections into output stream
     *
     *  @param[out]     out     the output stream
     */
    void dump(ostream & out);

    /*
     * Load the dictionary collections from input stream,
     * return true if dictionary successfully loaded, otherwise
     * false.
     *
     *  @param[in]      in      the input stream
     *  @return         bool    true on success, otherwise false.
     */
    bool load(istream & in);

    /*
     * Get the size of dictionary collections
     *
     *  @return         size_t  the size of the dictionary
     */
    size_t dim() const;

    /*
     * Retrieve the certain key in one of the dictionaries in this
     * collection. If create is specified, this key is created on
     * the condition that it is not in the dictionary. Return the 
     * index of the key, -1 on failure
     *
     *  @param[in]  tid     the index of the dictionary
     *  @param[in]  key     the key
     *  @param[in]  create  insert the key to dictionary if create
     *                      if true.
     *  @return     int     the index of the key, -1 on failure.
     */
    int retrieve(int tid, const char * key, bool create);

public:
    int idx;        /*< the shared index among dictionaries */

private:
    vector<Dictionary *> dicts;
};

// the dictionary class
// it's wrapper of class SmartMap<int>
class Dictionary {
public:
    Dictionary(DictionaryCollections * coll): 
        collections(coll) {}

    //StringMap<int>          database;
    SmartMap<int>           database;
    DictionaryCollections * collections;

    inline int retrieve(const char * key, bool create) {
        int val;

        if (database.get(key, val)) {
            return val;
        } else {
            if (create) {
                val = collections->idx;
                database.set(key, val);
                // database.unsafe_set(key, val);
                ++ collections->idx;
                return val;
            }
        }

        return -1;
    }

    inline int size() {
        return database.size();
    }
};

// labelcollections is a bi-direction map.
// it support two way of retrieving
//
//  * string key -> int index
//  * int index -> string key
//
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
    /*
     * push a key to the labelcollections
     *
     *  @param[in]  key     the key
     *  @return     int     the index of the key
     */
    // offsets of the key in hashmap key buffer is stored in entries.
    // when a new key is insert into the collection, check if entries
    // buffer is big enough. if not, duplicate the memory.
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

        return -1;
    }

    /*
     * get the key whose index is i
     *
     *  @param[in]  i               the index
     *  @return     const char *    pointer to the key
     */
    const char * at(int i) {
        if (i >= 0 && i < _num_entries) {
            return SmartMap<int>::_key_buffer + entries[i];
        } else {
            return 0;
        }
    }

    /*
     * get the index of the key. if the key doesn't exist, return -1
     *
     *  @param[in]  key             the key
     *  @return     int             index of the key if exist, otherwise -1
     */
    int index( const char * key ) {
        int val = -1;
        if (get(key, val)) {
            return val;
        }

        return -1;
    }

    /*
     * dump the collection to output stream
     *
     *  @param[out] out     the output stream
     */
    void dump(ostream & out) {
        SmartMap<int>::dump(out);
        out.write(reinterpret_cast<const char *>(entries), sizeof(int) * _num_entries);
    }

    /*
     * load the collections from input stream.
     *
     *  @param[in]  in      the input stream.
     *  @return     bool    true on success, otherwise false
     */
    bool load(istream & in) {
        bool ret = SmartMap<int>::load(in);
        if (!ret) {
            return ret;
        }

        if (entries) {
            delete [](entries);
        }

        entries = new int[SmartMap<int>::_num_entries];
        if (!entries) {
            return false;
        }

        in.read(reinterpret_cast<char *>(entries), sizeof(int) * _num_entries);
        return true;
    }
};

}       //  end for namespace parser
}       //  end for namespace ltp
#endif  //  end for __FEATURE_COLLECTIONS_H__
