#ifndef __FEATURE_VECTOR_H__
#define __FEATURE_VECTOR_H__

namespace ltp {
namespace parser {

struct FeatureVector {
    int * idx;
    double * val;
    int n;
};

}       //  end for namespace parser
}       //  end for namespace ltp

#endif  //  end for __FEATURE_VECTOR__
