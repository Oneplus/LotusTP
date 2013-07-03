#ifndef __TEMPLATE_HPP__
#define __TEMPLATE_HPP__

#include <iostream>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <hash_map>
#else
#include <ext/hash_map>
struct __Default_String_HashFunction{
    size_t operator()(const std::string& s) const {
        unsigned int _seed = 131; // 31 131 1313 13131 131313 etc..
        unsigned int _hash = 0;
        for(std::size_t i = 0; i < s.size(); i++) {
            _hash = (_hash * _seed) + s[i];
        }
        return size_t(_hash);
    }
};
#endif

namespace ltp {
namespace utility {

// a singleton used to story (token, id) mapping.
// A Template::Data should make a copy of the cache class.
template <typename T>
class __Template_Token_Cache {
public:
    static __Template_Token_Cache * get_cache() {
        if (0 == _instance) {
            _instance = new __Template_Token_Cache;
        }
        return _instance;
    }

    int push_back(const char * key) {
        // allocate new data
        int new_num;
        int len = strlen(key) + 1;

        for (int i = 0; i < _num_tokens; ++ i) {
            if (!strcmp(key, _tokens[i])) {
                return i;
            }
        }

        if (_cap_tokens <= (new_num = (_num_tokens + 1))) {
            _cap_tokens = (new_num << 1);

            char ** new_tokens = new char *[_cap_tokens];
            if (_tokens) {
                memcpy(new_tokens, _tokens, sizeof(char *)*_num_tokens);
                delete [](_tokens);
            }

            _tokens = new_tokens;
        }

        _tokens[_num_tokens] = new char[len];
        memcpy(_tokens[_num_tokens], key, len);
        ++ _num_tokens;
        return _num_tokens - 1;
    }

    const char * index(int idx) {
        if (idx < 0 || idx >= _num_tokens) {
            return 0;
        }

        return _tokens[idx];
    }

    int num_tokens() {
        return _num_tokens;
    }

    // this is shit that so many stars.
    void copy(char ** & tokens) {
        if (0 == tokens) {
            tokens = new char *[_num_tokens];
        }

        for (int i = 0; i < _num_tokens; ++ i) {
            int len = strlen(_tokens[i]) + 1;
            tokens[i] = new char[len];
            memcpy(tokens[i], _tokens[i], len);
        }
    }
protected:
    __Template_Token_Cache() :
        _tokens(0),
        _num_tokens(0),
        _cap_tokens(0) {}

private:
    static __Template_Token_Cache * _instance;

    char ** _tokens;
    int _num_tokens;
    int _cap_tokens;
};

template<typename T> __Template_Token_Cache<T> * __Template_Token_Cache<T>::_instance = NULL;

class Template {
public:
    class Data {
    public:
        Data() : _tokens(0) {
            _num_tokens = __Template_Token_Cache<void>::get_cache()->num_tokens();
            __Template_Token_Cache<void>::get_cache()->copy( _tokens );
        }

        ~Data() {
            for (int i = 0; i < _num_tokens; ++ i) {
                delete [](_tokens[i]);
            }
            delete [](_tokens);
        }

        bool set(const char * key, const char * val) {
            for (int i = 0; i < _num_tokens; ++ i) {
                // i didnt check case like "pid={pid}", for speed concerning.
                // users should guarantee no such template is used.
                if (!strcmp(_tokens[i], key)) {
                    int len = strlen(val) + 1;
                    char * new_key = new char[len];
                    memcpy(new_key, val, len);
                    delete [](_tokens[i]);
                    _tokens[i] = new_key;
                    return true;
                }
            }

            return false;
        }

        bool set(const char * key, const std::string & val) {
            return set( key, val.c_str() );
        }

        const char * index(int i) const {
            if (i < 0 || i >= _num_tokens) {
                return 0;
            }
            return _tokens[i];
        }

    private:
        char ** _tokens;
        int _num_tokens;
    };

public:
    Template(const char * tempstr) : items(0), buffer(0) {
        int len = strlen(tempstr);

        buffer = new char[len + 1];
        memcpy(buffer, tempstr, len + 1);

        num_items = 0;
        const char * s = NULL;

        int right_bracket = -1;
        int left_bracket = -1;
        for (s = buffer; *s; ++ s) {
            if ((*s) == '{') {
                left_bracket = s - buffer;
                if (right_bracket + 1 < left_bracket) {
                    ++ num_items;
                }
            }

            if ((*s) == '}') {
                right_bracket = s - buffer;
                ++ num_items;
            }
        }

        /*replacements = new int[num_items];
        items = new char *[num_items];*/
        items = new int[num_items];

        right_bracket = -1;
        int i = 0;
        for (s = buffer; *s; ++ s) {
            if ((*s) == '{') {
                left_bracket = s - buffer;
                if (right_bracket + 1 < left_bracket) {
                    //replacements[i] = -1;
                    //items[i] = buffer + right_bracket + 1;
                    buffer[left_bracket] = 0;
                    items[i] = __Template_Token_Cache<void>::get_cache()->push_back(
                            buffer + right_bracket + 1);
                    ++ i;
                }
            }

            if ((*s) == '}') {
                right_bracket = s - buffer;
                /*int idx = 0;
                for (int j = left_bracket + 1;
                        j < right_bracket;
                        ++ j) {
                    idx *= 10;
                    idx += (buffer[j] - '0');
                }*/

                buffer[right_bracket] = 0;
                // replacements[i] = idx;
                items[i] = __Template_Token_Cache<void>::get_cache()->push_back(
                        buffer + left_bracket + 1);

                ++ i;
            }
        }
    }

    ~Template() {
        /*if (replacements) {
            delete [](replacements);
        }*/

        if (items) {
            delete [](items);
        }

        if (buffer) {
            delete [](buffer);
        }
    }

    /*
     * Generate the template from template and 
     *
     *
     */
    inline bool render(const Data & data, std::string & ret) {
        ret.clear();

        for (int i = 0; i < num_items; ++ i) {
            /*if (replacements[i] >= 0) {
                ret.append( data.find( items[i] )->second );
            } else {
                ret.append( items[i] );
            }*/
            ret.append( data.index(items[i]) );
        }

        return true;
    }
private:
    int num_items;
    int * items;
    char * buffer;
    /*int * replacements;
    char ** items;*/
};
}       //  end for namespace utility
}       //  end for namespace ltp


#endif  //  end for __TEMPLATE_HPP__
