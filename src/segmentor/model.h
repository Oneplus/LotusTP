#ifndef __LTP_SEGMENTOR_MODEL_H__
#define __LTP_SEGMENTOR_MODEL_H__

#include "featurespace.h"
#include "parameter.h"

#include "smartmap.hpp"

namespace ltp {
namespace segmentor {

using namespace ltp::utility;

class Model {
public:
    Model();
    ~Model();

    inline int num_labels(void) {
        return labels.size();
    }

    void save(std::ostream & ofs);
    bool load(std::istream & ifs);

public:
    IndexableSmartMap   labels;
    FeatureSpace        space;
    Parameters          param;
};

}       //  end for namespace segmentor
}       //  end for namespace ltp

#endif  //  end for __LTP_SEGMENTOR_MODEL_H__
