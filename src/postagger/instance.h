#ifndef __LTP_POSTAGGER_INSTANCE_H__
#define __LTP_POSTAGGER_INSTANCE_H__

#include <iostream>
#include "featurevec.h"
#include "mat.h"
#include "sparsevec.h"

namespace ltp {
namespace postagger {

class Instance {
public:
    Instance() {}

    ~Instance() {
    }

    inline size_t size() const {
        return forms.size();
    }

    int num_errors() {
        int len = size();
        if ((len != tagsidx.size()) || (len != predicted_tagsidx.size())) {
            return -1;
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
        return size() - num_errors();
    }

public:
    std::vector< std::string > forms;
    std::vector< std::string > tags;
    std::vector< int >         tagsidx;
    std::vector< std::string > predicted_tags;
    std::vector< int >         predicted_tagsidx;

    math::SparseVec             features;                   /*< the gold features */
    math::SparseVec             predicted_features;         /*< the predicted features */

    math::Mat< FeatureVector *> uni_features;
    math::Mat< double > uni_scores;
    math::Mat< double > bi_scores;

    std::vector< std::vector< std::string> > chars;
};

}       //  end for namespace postagger
}       //  end for namespace ltp

#endif  //  end for __LTP_POSTAGGER_INSTANCE_H__
