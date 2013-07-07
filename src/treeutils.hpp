#ifndef __TREE_UTILS_HPP__
#define __TREE_UTILS_HPP__

#include <iostream>
#include <vector>
#include <list>

#include <string.h>

namespace ltp {
namespace parser {
namespace treeutils {

/*
 * Get children given certain heads, complexity of is O(n)
 *
 *  @param[in]  heads           the heads
 *  @param[out] children_left   result for left children
 *  @param[out] children_right  result for right children
 */
inline int get_children( const std::vector<int> & heads,
        std::vector< std::list<int> > & children_left,
        std::vector< std::list<int> > & children_right ) {
    int ret = 0;
    int len = heads.size();

    children_left.resize(len);
    children_right.resize(len);

    // clear each element in the output vector
    for (int i = 0; i < len; ++ i) {
        children_left[i].clear();
        children_right[i].clear();
    }

    for (int i = 1; i < len; ++ i) {
        int hid = heads[i];

        if (i < hid) {
            ret ++;
            children_left[hid].push_front(i);
        } else {
            children_right[hid].push_back(i);
        }
    }

    return ret;
}

// Generate all the tree space in dependency
// This class is a Python `yield` like generator
// detail for implement can refer to 
// http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html
class DEPTreeSpaceIterator {
public:
    DEPTreeSpaceIterator(int len) : 
        _len(len),
        _hid(0), 
        _cid(0),
        _state(0) {
        ++ (*this);
    }

    inline int hid(void) {
        return _hid;
    }

    inline int cid(void) {
        return _cid;
    }

    inline bool end(void) {
        return _hid >= _len;
    }

    void operator ++(void) {
        switch (_state) {
            case 0:
                for (_hid = 0; _hid < _len; ++ _hid) {
                    for (_cid = 0; _cid < _len; ++ _cid) {
                        if (_hid == _cid) {
                            continue;
                        }
                        _state = 1;
                        return;
            case 1:;
                    }
                }
        }
    }
private:
    int _len;
    int _hid;
    int _cid;
    int _state;
};      //  end for DEPIterator

// Generate all the tree space given a tree.
class SIBTreeSpaceIterator {
public:
    SIBTreeSpaceIterator(int len, bool last_sibling = true) :
        _len(len),
        _last_sibling(last_sibling),
        _hid(0),
        _cid(0),
        _sid(0), 
        _step(0),
        _end(0),
        _state(0) {
        ++ (*this);
    }

    inline int hid() {
        return _hid;
    }

    inline int cid() {
        return _cid;
    }

    inline int sid() {
        return _sid;
    }

    inline bool end() {
        return _hid >= _len;
    }

    void operator ++(void) {

        switch (_state) {
            case 0:
                for (_hid = 0; _hid < _len; ++ _hid) {
                    for (_cid = 0; _cid < _len; ++ _cid) {
                        if (_hid == _cid) {
                            continue;
                        }

                        _step = (_hid < _cid ? 1 : -1);
                        _end = (_last_sibling ? _cid + _step : _cid);
                        for (_sid = _hid; _sid != _end; _sid += _step) {
                            _state = 1;
                            return;
            case 1:;
                        }
                    }
                }
        }
    }
private:
    int _len;
    int _hid;
    int _cid;
    int _sid;
    int _step;
    int _end;
    int _state;
    bool _last_sibling;
};

class DEPIterator {
public:
    DEPIterator(const std::vector<int> & heads) : 
        _cid(1),
        _len(heads.size()),
        _heads(heads) {}

    inline int hid() {
        return _heads[_cid];
    }

    inline int cid() {
        return _cid;
    }

    inline bool end() {
        return _cid >= _len;
    }

    void operator ++(void) {
        ++ _cid;
    }
private:
    int _len;
    int _cid;
    const std::vector<int> & _heads;
};

class SIBIterator {
public:
    SIBIterator(const std::vector<int> & heads, bool last_sibling = true) : 
        _hid(0),
        _state(0),
        _last_sibling(last_sibling),
        _len(heads.size()),
        _heads(heads) {

        for (int dir = 0; dir < 2; ++ dir) {
            _children[dir]  = new int *[_len];
            _num_children[dir] = new int[_len];

            memset(_num_children[dir], 0, sizeof(int) * _len);

            for (int i = 0; i < _len; ++ i) {
                _children[dir][i]  = new int[_len];
                _children[dir][i][_num_children[dir][i] ++] = i;
            }
        }

        for (int i = _len - 1; i > 0; -- i) {
            int hid = _heads[i];
            int * children = _children[0][hid];
            if (i < hid) {
                children[_num_children[0][hid] ++] = i;
            }
        }

        for (int i = 1; i < _len; ++ i) {
            int hid = _heads[i];
            int * children = _children[1][hid];
            if (i > hid) {
                children[_num_children[1][hid] ++] = i;
            }
        }

        if (_last_sibling) {
            for (int i = 0; i < _len; ++ i) {
                for (int dir = 0; dir < 2; ++ dir) {
                    if (_num_children[dir][i] > 1) {
                        _children[dir][i][_num_children[dir][i]] = _children[dir][i][_num_children[dir][i] - 1];
                        _num_children[dir][i] ++;
                    }
                }
            }
        }

        ++ (*this);
    }

    ~SIBIterator() {
        for (int i = 0; i < _len; ++ i) {
            delete _children[0][i];
            delete _children[1][i];
        }

        delete _children[0];
        delete _children[1];
    }

    inline int hid(void) {
        return _hid;
    }

    inline int cid(void) {
        return _cid;
    }

    inline int sid(void) {
        return _sid;
    }

    inline bool end(void) {
        return _hid >= _len;
    }

    void operator ++(void) {

        switch (_state) {
            case 0:
                for (_hid = 0; _hid < _len; ++ _hid) {
                    for (_dir = 0; _dir < 2; ++ _dir) {
                        for (_idx = 1; _idx < _num_children[_dir][_hid]; ++ _idx) {
                            _cid = _children[_dir][_hid][_idx];
                            _sid = _children[_dir][_hid][_idx - 1];
                            _state = 1;
                            return;
            case 1:;
                        }
                    }
                }
        }
    }
private:
    void debug(void) {
        for (int i = 0; i < _len; ++ i) {
            std::cerr << "[" << i << "] --> (";
            for (int j = 1; j < _num_children[0][i]; ++ j) {
                std::cerr << _children[0][i][j] << ",";
            }
            std::cerr << "), (";
            for (int j = 1; j < _num_children[1][i]; ++ j) {
                std::cerr << _children[1][i][j] << ",";
            }
            std::cerr << ")" << std::endl;
        }
    }

private:
    int _hid;
    int _cid;
    int _sid;
    int _dir;
    int _len;
    int _idx;
    int _state;
    bool _last_sibling;
    const std::vector<int> & _heads;

    int ** _children[2];
    int *  _num_children[2];
};

}       //  end for namespace treeutils
}       //  end for namespace parser
}       //  end for namespace ltp

#endif  //  end for __TREE_UTILS_HPP__
