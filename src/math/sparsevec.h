#ifndef __SPARSE_VECTOR_H__
#define __SPARSE_VECTOR_H__

#include "featurevec.h"

#ifdef _WIN32
#include <hash_map>
#else
//#include <ext/hash_map>
#include <tr1/unordered_map>
#endif

namespace ltp {
namespace math {

using namespace ltp::parser;

class SparseVec {
public:
#ifdef _WIN32
    typedef stdext::hash_map<int,double> internal_sparsevec_t;
#else
    typedef std::tr1::unordered_map<int, double> internal_sparsevec_t;
    // typedef __gnu_cxx::hash_map<int, double> internal_sparsevec_t;
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

    inline int dim() const {
        return _vec.size();
    }

    inline double L2() {
        double norm = 0;
        for (const_iterator itx = _vec.begin();
                itx != _vec.end(); ++ itx) {
            double val = itx->second;
            norm += (val * val);
        }
        return norm;
    }

    inline void add(const SparseVec &other,
            const double scale) {
        for (const_iterator itx = other.begin();
                itx != other.end(); ++ itx) {
            int idx = itx->first;
            if (_vec.find(idx) == _vec.end()) _vec[idx] = 0.;
            _vec[idx] += (scale * itx->second);
        }
    }

    inline void add(const FeatureVector * other,
            const double scale) {
        if (!other) {
            return;
        }
        int n = other->n;
        const int * idx = other->idx;
        const double * val = other->val;

        if (val == NULL) {
            for (int i = 0; i < n; ++ i) {
                if (_vec.find(idx[i]) == _vec.end()) _vec[idx[i]] = 0.;
                _vec[idx[i]] += scale;
            }
        } else {
            for (int i = 0; i < n; ++ i) {
                _vec[idx[i]] += (scale * val[i]);
            }
        }
    }

    inline void zero() {
        _vec.clear();
    }

    inline void str(ostream & out, string prefix = "  ") const {
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
