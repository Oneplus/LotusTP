#ifndef __INSTANCE_H__
#define __INSTANCE_H__

#include <iostream>
#include <set>

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
        if (labeled_dependency_features.total_size() > 0) {
            for (int i = 0; i < labeled_dependency_features.dim1(); ++ i) {
                for (int j = 0; j < labeled_dependency_features.dim2(); ++ j) {
                    for (int k = 0; k < labeled_dependency_features.dim3();++ k) {
                        delete labeled_dependency_features[i][j][k];
                    }
                }
            }
        }

        if (dependency_features.total_size() > 0) {
            for (int i = 0; i < dependency_features.nrows(); ++ i) {
                for (int j = 0; j < dependency_features.ncols(); ++ j) {
                    delete dependency_features[i][j];
                }
            }
        }
    }

    size_t size() const {
        return forms.size();
    }

    int num_error_heads(bool ignore_punctation = true) const {
        if (predicted_heads.size() == 0) {
            return size();
        }

        int ret = 0;
        int len = size();
        for (int i = 1; i < len; ++ i) {
            if (ignore_punctation && postags[i] == "wp") {
                continue;
            }

            if (predicted_heads[i] != heads[i]) {
                ++ ret;
            }
        }

        return ret;
    }

    int num_error_labels(bool ignore_punctation = true) {
        if (predicted_heads.size() == 0 || predicted_deprelsidx.size() == 0) {
            return size();
        }

        int ret = 0;
        int len = size();
        for (int i = 1; i < len; ++ i) {
            if (ignore_punctation && postags[i] == "wp") {
                continue;
            }

            if (predicted_heads[i] == heads[i] && predicted_deprelsidx[i] != deprelsidx[i]) {
                ++ ret;
            }
        }

        return ret;
    }

    double num_errors() {
        return num_error_heads() + 0.5 * num_error_labels();
    }

    int num_correct_heads(bool ignore_punctation = true) {
        if (predicted_heads.size() == 0) {
            return 0;
        }

        int ret = 0;
        int len = size();
        for (int i = 1; i < len; ++ i) {
            if (ignore_punctation && postags[i] == "wp") {
                continue;
            }

            if (predicted_heads[i] == heads[i]) {
                ++ ret;
            }
        }

        return ret;
    }

    int num_correct_heads_and_labels(bool ignore_punctation = true) {
        if (predicted_heads.size() == 0 || predicted_deprelsidx.size() == 0) {
            return 0;
        }

        int ret = 0;
        int len = size();
        for (int i = 1; i < len; ++ i) {
            if (ignore_punctation && postags[i] == "wp") {
                continue;
            }

            if (predicted_heads[i] == heads[i] && (predicted_deprelsidx[i] == deprelsidx[i])) {
                ++ ret;
            }
        }

        return ret;
    }

    int num_rels(bool ignore_punctation = true) const {
        if (!ignore_punctation) {
            return forms.size() - 1;
        } else {
            int ret = 0;
            int len = size();
            for (int i = 1; i < len; ++ i) {
                if (postags[i] != "wp") {
                    ++ ret;
                }
            }
            return ret;
        }
        return -1;
    }
public:

    vector<string>  forms;      /* */
    vector<string>  lemmas;     /* */
    vector< vector<string> > chars;
    vector<string>  postags;    /* */

    vector<int>     heads;
    vector<int>     deprelsidx;
    vector<string>  deprels;
    SparseVec       features;

    vector<int>     predicted_heads;
    vector<int>     predicted_deprelsidx;
    vector<string>  predicted_deprels;
    SparseVec       predicted_features;

    /* features group */
    Vec<FeatureVector *>    postag_unigram_features;
    Vec<double>             postag_unigram_scores;

    Mat<FeatureVector *>    dependency_features;
    Mat<double>             dependency_scores;

    Mat3<FeatureVector *>   labeled_dependency_features;
    Mat3<double>            labeled_dependency_scores;

    Mat3<FeatureVector *>   sibling_features;
    Mat3<double>            sibling_scores;

    Mat4<FeatureVector *>   labeled_sibling_features;
    Mat4<double>            labeled_sibling_scores;

    vector<int>             verb_cnt;
    vector<int>             conj_cnt;
    vector<int>             punc_cnt;
};  // end for class Instance
}   // end for namespace parser
}   // end for namespace ltp

#endif  // end for __INSTANCE_H__
