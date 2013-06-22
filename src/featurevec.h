#ifndef __FEATURE_VECTOR_H__
#define __FEATURE_VECTOR_H__

namespace ltp {
namespace parser {

struct FeatureVector {
    FeatureVector() : n(0), idx(0), val(0) {}
    ~FeatureVector() {
        if (idx) {
            delete [](idx);
        }

        if (val) {
            delete [](idx);
        }
    }
    int * idx;
    double * val;
    int n;
};

}       //  end for namespace parser
}       //  end for namespace ltp

#endif  //  end for __FEATURE_VECTOR__
