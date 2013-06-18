#ifndef __SPARSE_VECTOR_H__
#define __SPARSE_VECTOR_H__

#include "featurevec.h"

#ifdef _WIN32
#include <hash_map>
#else
#include <ext/hash_map>
#endif

namespace ltp {
namespace math {

using namespace ltp::parser;

class SparseVec {
public:
#ifdef _WIN32
    typedef stdext::hash_map<int,double> internal_sparsevec_t;
#else
    typedef __gnu_cxx::hash_map<int, double> internal_sparsevec_t;
#endif  //  end for _WIN32
    typedef internal_sparsevec_t::iterator       iterator;
    typedef internal_sparsevec_t::const_iterator const_iterator;

    SparseVec() {}
    ~SparseVec() {}

    const_iterator begin() const {
        return _vec.begin();
    }

    const_iterator end() const {
        return _vec.end();
    }

    iterator mbegin() {
        return _vec.begin();
    }

    iterator mend() {
        return _vec.end();
    }

    int dim() const {
        return _vec.size();
    }

    double L2() {
        double norm = 0;
        for (const_iterator itx = _vec.begin();
                itx != _vec.end(); ++ itx) {
            double val = itx->second;
            norm += (val * val);
        }
        return norm;
    }

    void add(const SparseVec &other,
            const double scale = 1.0) {
        for (const_iterator itx = other.begin();
                itx != other.end(); ++ itx) {
            int idx = itx->first;
            _vec[idx] += (scale * itx->second);
        }
    }

    void add(const FeatureVector * other,
            const double scale = 1.0) {
        int n = other->n;
        const int * idx = other->idx;
        const double * val = other->val;

        if (val == NULL) {
            for (int i = 0; i < n; ++ i) {
                _vec[idx[i]] += scale;
            }
        } else {
            for (int i = 0; i < n; ++ i) {
                _vec[idx[i]] += (scale * val[i]);
            }
        }
    }

    void zero() {
        _vec.clear();
    }

    void str(ostream & out, string prefix = "  ") const {
        int i = 0;
        out << "{ ";
        for (const_iterator itx = _vec.begin();
                itx != _vec.end();
                ++ itx) {
            out << itx->first << ":" << itx->second << ", ";
            ++ i;

            if (i % 10 == 0) {
                out << "\n" << prefix;
            }
        }
        out << "}" << endl;
    }
private:
    internal_sparsevec_t _vec;
    double norm;
};

}       //  end for namespace math
}       //  end for namespace ltp

#endif  //  end for __SPARSE_VECTOR_H__