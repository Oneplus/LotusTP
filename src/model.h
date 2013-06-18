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

    void save(ostream & out) {
        out << "basic:{";
        out << "labeled:"                   << model_opt.labeled << ",";
        out << "decoder-name:"              << model_opt.decoder_name << ",";
        out << "use-dependency:"            << feat_opt.use_dependency << ",";
        out << "use-dependency-unigram:"    << feat_opt.use_dependency_unigram << ",";
        out << "use-dependency-bigram:"     << feat_opt.use_dependency_bigram << ",";
        out << "use-dependency-surrounding:"<< feat_opt.use_dependency_surrounding << ",";
        out << "use-dependency-between:"    << feat_opt.use_dependency_between << ",";
        out << "postags: {";
        out << "},";
        out << "deprels: {";
        out << "},";
        out << "features-dimension:"        << collections.dim() << ",";
        out << "},";
        out << "features:{";                collections.dump(out);
        out << "},";
        out << "parameters:{";              param.dump(out);
        out << "}";
    }

};      //  end for class model

}       //  end for namespace parser
}       //  end for namespace ltp
#endif  //  end for __MODEL_H__
