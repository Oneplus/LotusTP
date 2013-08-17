#include "py_segmentor.hpp"
#include "segment_dll.h"

Segmentor::Segmentor( std::string model_path ) {
    engine = segmentor_create_segmentor( model_path.c_str() );
}

Segmentor::~Segmentor() {
    segmentor_release_segmentor( engine );
}

std::vector< std::string > Segmentor::segment( std::string line ) {
    std::vector< std::string > ret;

    segmentor_segment( engine, line, ret );

    return ret;
}
