#ifndef __LTP_NER_INSTANCE_H__
#define __LTP_NER_INSTANCE_H__

#include <iostream>
#include "featurevec.h"
#include "mat.h"
#include "sparsevec.h"

namespace ltp {
namespace ner {

class Instance {
public:
    Instance() {}

    ~Instance() {
        cleanup();
    }

    inline size_t size() const {
        return forms.size();
    }

    int num_errors() {
        int len = size();
        if ((len != tagsidx.size()) || (len != predicted_tagsidx.size())) {
            return len;
        }

        int ret = 0;
        for (int i = 0; i < len; ++ i) {
            if (tagsidx[i] != predicted_tagsidx[i]) {
                ++ ret;
            }
        }

        return ret;
    }

    int num_corrected_predicted_tags() {
        int len = size();
        int ret = 0;

        for (int i = 0; i < len; ++ i) {
            if (tagsidx[i] == predicted_tagsidx[i]) {
                ++ ret;
            }
        }

        return ret;
    }

    int num_gold_ne() {
        return ne.size();
    }

    int num_predicted_ne() {
        return predicted_ne.size();
    }

    int num_recalled_ne() {
        int len = 0;
        int ret = 0;
        int gold_len = 0, predicted_len = 0;

        for (int i = 0; i < ne.size(); ++ i) {
            len += ne[i].size();
        }

        for (int i = 0, j = 0; i < ne.size() && j < predicted_ne.size(); ) {
            if (ne[i] == predicted_ne[j]) {
                ++ ret;
                gold_len += ne[i].size();
                predicted_len += predicted_ne[j].size();

                ++ i;
                ++ j;
            } else {
                gold_len += ne[i].size();
                predicted_len += predicted_ne[j].size();

                ++ i;
                ++ j;

                while (gold_len < len && predicted_len < len) {
                    if (gold_len < predicted_len) {
                        gold_len += ne[i].size();
                        ++ i;
                    } else if (gold_len > predicted_len) {
                        predicted_len += predicted_ne[j].size();
                        ++ j;
                    } else {
                        break;
                    }
                }
            }
        }

        return ret;
    }

    int cleanup() {
        int len = 0;
        if ((len = uni_features.total_size()) > 0) {
            int d1 = uni_features.nrows();
            int d2 = uni_features.ncols();

            for (int i = 0; i < d1; ++ i) {
                if (uni_features[i][0]) {
                    uni_features[i][0]->clear();
                }
                for (int j = 0; j < d2; ++ j) {
                    if (uni_features[i][j]) {
                        delete uni_features[i][j];
                    }
                }
            }
        }

        uni_features.dealloc();
        uni_scores.dealloc();
        bi_scores.dealloc();

        features.zero();
        predicted_features.zero();

        return 0;
    }
public:
    std::vector< std::string >  raw_forms;
    std::vector< std::string >  forms;
    std::vector< std::string >  postags;
    std::vector< std::string >  tags;
    std::vector< int >          tagsidx;
    std::vector< std::string >  predicted_tags;
    std::vector< int >          predicted_tagsidx;
    std::vector< std::string >  ne;
    std::vector< std::string >  predicted_ne;

    math::SparseVec             features;                   /*< the gold features */
    math::SparseVec             predicted_features;         /*< the predicted features */

    math::Mat< FeatureVector *> uni_features;
    math::Mat< double >         uni_scores;
    math::Mat< double >         bi_scores;
};

}       //  end for namespace ner
}       //  end for namespace ltp

#endif  //  end for __LTP_NER_INSTANCE_H__
