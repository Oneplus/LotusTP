#ifndef __LTP_PYTHON_WRAPPER_HPP__
#define __LTP_PYTHON_WRAPPER_HPP__

#include <iostream>
#include <vector>

class Segmentor {
public:
    Segmentor( std::string model_path);
    ~Segmentor();
    std::vector< std::string > segment( std::string line );

private:
    void * engine;
};

class Postagger {
public:
    Postagger( std::string model_path);
    ~Postagger();
    std::vector< std::string > postag( std::vector< std::string > line );

private:
    void * engine;
};

class NER {
public:
    NER( std::string model_path);
    ~NER();
    std::vector< std::string > recognize(
            std::vector< std::string > words,
            std::vector< std::string > postags );

private:
    void * engine;
};

struct ParseResult {
    std::vector< int > heads;
    std::vector< std::string > deprels;
};

class Parser {
public:
    Parser( std::string model_path );
    ~Parser();
    ParseResult parse(
            std::vector< std::string > words,
            std::vector< std::string > postags);
private:
    void * engine;
};

#endif  //  end for __SEGMENT_HPP__
