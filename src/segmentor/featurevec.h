#ifndef __LTP_SEGMENTOR_FEATURE_VECTOR_H__
#define __LTP_SEGMENTOR_FEATURE_VECTOR_H__

namespace ltp {
namespace segmentor {

struct FeatureVector {
public:
    FeatureVector () : n(0), idx(0), val(0) {
    }

    ~FeatureVector() {
        if (idx) {
            delete [](idx);
        }

        if (val) {
            delete [](val);
        }
    }

public:
    int      n;
    int *    idx;
    double * val;
    int      loff;
};

}       //  end for namespace segmentor
}       //  end for namespace ltp

#endif  //  end for __LTP_SEGMENTOR_FEATRUE_VECTOR_H__
