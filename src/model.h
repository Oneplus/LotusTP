#ifndef __MODEL_H__
#define __MODEL_H__

#include "collections.h"
#include "parameters.h"

namespace ltp {
namespace parser {

class Model {
public:
    Model() {}
    ~Model() {}

public:
    DictionaryCollections   collections;
    Parameters              param;

};      //  end for class model

}       //  end for namespace parser
}       //  end for namespace ltp
#endif  //  end for __MODEL_H__
