#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

namespace ltp {
namespace parser {

class Parameters {
public:
    int _dim;
    double * _W;
    double * _W_sum;
    int *    _W_time;

    Parameters() :
        _dim(-1),
        _W(0),
        _W_sum(0),
        _W_time(0) {}

    void realloc(int dim) {
        dealloc();
        _dim = dim;

        if (dim > 0) {
            _W      = new double[dim];
            _W_sum  = new double[dim];
            _W_time = new int[dim];
        }

        for (int i = 0; i < dim; ++ i) {
            _W[i]       = 0;
            _W_sum[i]   = 0;
            _W_time[i]  = 0;
        }
    }

    void dealloc() {
        if (_W) {
            delete [](_W);
            _W = 0;
        }
        if (_W_sum) {
            delete [](_W_sum);
            _W_sum = 0;
        }
        if (_W_time) {
            delete [](_W_time);
            _W_time = 0;
        }
    }

    double operator[] (int idx) const{
        return _W_sum[idx];
    }

    void add(const SparseVec & vec, int now, double scale = 1.) {
        for (SparseVec::const_iterator itx = vec.begin();
                itx != vec.end();
                itx ++) {
            int idx = itx->first;
            double upd = scale * itx->second;
            double cur_val = _W[idx];
            int elapsed = now - _W_time[idx];
            _W[idx] = cur_val + upd;
            _W_sum[idx] += elapsed * cur_val + upd;
            _W_time[idx] = now;
        }
    }

    double dot(const SparseVec & vec, bool use_avg = true) const {
        const double * const p = (use_avg ? _W_sum : _W);
        double ret = 0.;
        for (SparseVec::const_iterator itx = vec.begin();
                itx != vec.end();
                ++ itx) {
            ret += p[itx->first] * itx->second;
        }
        return ret;
    }

    double dot(const FeatureVector * vec, bool use_avg = true) const {
        const double * const p = (use_avg ? _W_sum : _W);
        double ret = 0.;
        for (int i = 0; i < vec->n; ++ i) {
            if (vec->val) {
                ret += p[vec->idx[i]] * vec->val[i];
            } else {
                ret += p[vec->idx[i]];
            }
        }
        return ret;
    }


    void divide(double denomitor) {
        for (int i = 0; i < _dim; ++ i) {
            _W[i] = _W[i] / denomitor;
        }
    }

    void str(ostream& out, int width = 10) {
        out << "\t";
        for (int i = 0; i < width; ++ i) {
            out << "[" << i << "]\t";
        }
        out << endl;
        for (int i = 0; i < _dim; ++ i) {
            if (i % width == 0) {
                out << "[" << i << "-" << (i / width + 1)  * width - 1 << "]\t";
            }
            out << _W[i] << "\t";
            if ((i + 1) % width == 0) {
                out << endl;
            }
        }
        out << endl;
    }
};

}   //  end for namespace parser
}   //  end for namespace ltp

#endif  //  end for __PARAMETERS_H__
