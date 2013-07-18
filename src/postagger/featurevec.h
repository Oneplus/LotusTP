#ifndef __LTP_POSTAGGER_FEATURE_VECTOR_H__
#define __LTP_POSTAGGER_FEATURE_VECTOR_H__

namespace ltp {
namespace postagger {

struct FeatureVector {
public:
    FeatureVector () : n(0), idx(0), val(0) {
    }

    ~FeatureVector() {
    }

    void clear() {
        if (idx) {
            delete [](idx);
            idx = 0;
        }

        if (val) {
            delete [](val);
            val = 0;
        }
    }


public:
    int      n;
    int *    idx;
    double * val;
    int      loff;
};

}       //  end for namespace postagger
}       //  end for namespace ltp

#endif  //  end for __LTP_POSTAGGER_FEATRUE_VECTOR_H__
