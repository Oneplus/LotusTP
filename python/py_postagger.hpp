#ifndef __LTP_PYTHON_WRAPPER_POSTAGGER_HPP__
#define __LTP_PYTHON_WRAPPER_POSTAGGER_HPP__

#include <iostream>
#include <vector>

class Postagger {
public:
    Postagger( std::string model_path);
    ~Postagger();
    std::vector< std::string > postag( std::vector< std::string > line );

private:
    void * engine;
};

#endif  //  end for __LTP_PYTHON_WRAPPER_POSTAGGER_HPP__
