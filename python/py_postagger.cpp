#include "py_postagger.hpp"
#include "postag_dll.h"

Postagger::Postagger( std::string model_path ) {
    engine = postagger_create_postagger( model_path.c_str() );
}

Postagger::~Postagger() {
    postagger_release_postagger( engine );
}

std::vector< std::string > Postagger::postag( std::vector< std::string > words ) {
    std::vector< std::string > ret;

    postagger_postag( engine, words, ret );

    return ret;
}
