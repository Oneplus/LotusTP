#ifndef __INSTANCE_H__
#define __INSTANCE_H__

#include <iostream>

#include "mat.h"
#include "sparsevec.h"

#include "featurevec.h"

namespace ltp {
namespace parser {

using namespace std;
using namespace ltp::math;

class Instance {
public:
    Instance(){
    }

    ~Instance() {
    }

    size_t size() const {
        return forms.size();
    }
public:

    vector<string>  forms;      /* */
    vector<string>  lemmas;     /* */
    vector< vector<string> > chars;
    vector<string>  postags;    /* */

    vector<int>     heads;
    vector<string>  deprels;
    SparseVec       features;

    vector<int>     predicted_heads;
    vector<string>  predicted_deprels;
    SparseVec       predicted_features;

    /* features group */
    Vec<FeatureVector *>    postag_unigram_features;
    Vec<double>             postag_unigram_scores;

    Mat<FeatureVector *>    dependency_features;
    Mat<double>             dependency_scores;

    Mat3<FeatureVector *>   labeled_dependency_features;
    Mat3<double>            labeled_dependency_scores;
};  // end for class Instance
}   // end for namespace parser
}   // end for namespace ltp

#endif  // end for __INSTANCE_H__
