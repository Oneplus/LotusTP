#ifndef __LTP_PYTHON_WRAPPER_SEGMENT_HPP__
#define __LTP_PYTHON_WRAPPER_SEGMENT_HPP__

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

#endif  //  end for __SEGMENT_HPP__
