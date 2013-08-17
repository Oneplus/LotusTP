#include "pyltp.hpp"
#include "segment_dll.h"
#include "postag_dll.h"
#include "ner_dll.h"
#include "parser_dll.h"

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

NER::NER( std::string model_path ) {
    engine = ner_create_recognizer( model_path.c_str() );
}

NER::~NER() {
    ner_release_recognizer( engine );
}

std::vector< std::string > NER::recognize(
        std::vector< std::string > words,
        std::vector< std::string > postags) {
    std::vector< std::string > ret;

    ner_recognize( engine, words, postags, ret );

    return ret;
}

Parser::Parser( std::string model_path ) {
    engine = parser_create_parser( model_path.c_str() );
}

Parser::~Parser() {
    parser_release_parser( engine );
}

ParseResult Parser::parse(
        std::vector< std::string > words,
        std::vector< std::string > postags ) {
    ParseResult ret;

    parser_parse(engine,
            words,
            postags,
            ret.heads, 
            ret.deprels);

    return ret;
}
