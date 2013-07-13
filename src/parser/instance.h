#ifndef __INSTANCE_H__
#define __INSTANCE_H__

#include <iostream>
#include <set>

#include "mat.h"
#include "sparsevec.h"
#include "featurevec.h"

namespace ltp {
namespace parser {

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

        if (labeled_sibling_features.total_size() > 0) {
            for (int i = 0; i < labeled_sibling_features.dim1(); ++ i) {
                for (int j = 0; j < labeled_sibling_features.dim2(); ++ j) {
                    for (int k = 0; k < labeled_sibling_features.dim3(); ++ k) {
                        for (int m = 0; m < labeled_sibling_features.dim4(); ++ m) {
                            delete labeled_sibling_features[i][j][k][m];
                        }
                    }
                }
            }
        }

        if (sibling_features.total_size() > 0) {
            for (int i = 0; i < sibling_features.dim1(); ++ i) {
                for (int j = 0; j < sibling_features.dim2(); ++ j) {
                    for (int k = 0; k < sibling_features.dim3(); ++ k) {
                        delete sibling_features[i][j][k];
                    }
                }
            }
        }
    }

    /*
     * Get length of the instance. Instance's length is defined as number
     * of form tokens.
     *
     *  @return     size_t              the number of tokens
     */
    size_t size() const {
        return forms.size();
    }

    /*
     * Get number of error of the heads. Prepositive condition is number 
     * of predicted_heads greater than 0 and number of predicated_heads
     * equals number of heads.
     *
     *  @param[in]  ignore_punctation   specify whether ignore punction
     *  @return     int                 the number of errors
     */
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

    int dump_all_featurevec(std::ostream & ofs) {
        int len;
        FeatureVector ** fvs = 0;
        if ((len = dependency_features.total_size()) > 0 && 
                (fvs = dependency_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->write(ofs);
                    fvs[i]->nice();
                }
            }
        }

        if ((len = labeled_dependency_features.total_size()) > 0 && 
                (fvs = labeled_dependency_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->write(ofs);
                    fvs[i]->nice(); 
                }
            }
        }

        if ((len = sibling_features.total_size()) > 0 &&
                (fvs = sibling_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->write(ofs);
                    fvs[i]->nice();
                }
            }
        }

        if ((len = labeled_sibling_features.total_size()) > 0 &&
                (fvs = labeled_sibling_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->write(ofs);
                    fvs[i]->nice();
                }
            }
        }
    }

    int load_all_featurevec(std::istream & ifs) {
        int len;
        FeatureVector ** fvs = 0;
        if ((len = dependency_features.total_size()) > 0 && 
                (fvs = dependency_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->read(ifs);
                }
            }
        }

        if ((len = labeled_dependency_features.total_size()) > 0 && 
                (fvs = labeled_dependency_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->read(ifs);
                }
            }
        }

        if ((len = sibling_features.total_size()) > 0 &&
                (fvs = sibling_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->read(ifs);
                }
            }
        }

        if ((len = labeled_sibling_features.total_size()) > 0 &&
                (fvs = labeled_sibling_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->read(ifs);
                }
            }
        }
    }

    int nice_all_featurevec() {
        int len;
        FeatureVector ** fvs;
        if ((len = dependency_features.total_size()) > 0 && 
                (fvs = dependency_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->nice();
                }
            }
        }

        if ((len = labeled_dependency_features.total_size()) > 0 && 
                (fvs = labeled_dependency_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->nice();
                }
            }
        }

        if ((len = sibling_features.total_size()) > 0 &&
                (fvs = sibling_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->nice();
                }
            }
        }

        if ((len = labeled_sibling_features.total_size()) > 0 &&
                (fvs = labeled_sibling_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->nice();
                }
            }
        }
    }

    int cleanup() {
        int len;
        FeatureVector ** fvs;
        if ((len = dependency_features.total_size()) > 0 && 
                (fvs = dependency_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->nice();
                    delete fvs[i];
                }
            }
        }

        if ((len = labeled_dependency_features.total_size()) > 0 && 
                (fvs = labeled_dependency_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->nice();
                    delete fvs[i];
                }
            }
        }

        if ((len = sibling_features.total_size()) > 0 &&
                (fvs = sibling_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->nice();
                    delete fvs[i];
                }
            }
        }

        if ((len = labeled_sibling_features.total_size()) > 0 &&
                (fvs = labeled_sibling_features.c_buf())) {
            for (int i = 0; i < len; ++ i) {
                if (fvs[i]) {
                    fvs[i]->nice();
                    delete fvs[i];
                }
            }
        }
        features.zero();
        predicted_features.zero();
        dependency_features.dealloc();
        dependency_scores.dealloc();
        labeled_dependency_features.dealloc();
        labeled_dependency_scores.dealloc();
        sibling_features.dealloc();
        sibling_scores.dealloc();
        labeled_sibling_features.dealloc();
        labeled_sibling_scores.dealloc();
    }
public:

    std::vector< std::string >                  forms;      /*< the forms */
    std::vector< std::string >                  lemmas;     /*< the lemmas */
    std::vector< std::vector< std::string> >    chars;      /*< the characters */
    std::vector< std::string >                  postags;    /*< the postags */

    std::vector<int>                            heads;
    std::vector<int>                            deprelsidx;
    std::vector< std::string >                  deprels;
    std::vector<int>                            predicted_heads;
    std::vector<int>                            predicted_deprelsidx;
    std::vector< std::string >                  predicted_deprels;

    SparseVec               predicted_features;
    SparseVec               features;

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

    std::vector<int>             verb_cnt;
    std::vector<int>             conj_cnt;
    std::vector<int>             punc_cnt;
};  // end for class Instance
}   // end for namespace parser
}   // end for namespace ltp

#endif  // end for __INSTANCE_H__
