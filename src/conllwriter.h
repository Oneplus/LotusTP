#ifndef __CONLL_WRITER_H__
#define __CONLL_WRITER_H__

#include <iostream>
#include <fstream>

#include "strutils.hpp"
#include "instance.h"

namespace ltp {
namespace parser {
using namespace std;
using namespace ltp::strutils;

class CoNLLWriter {
public:
    CoNLLWriter(ofstream& _f): f(_f)  {}
    ~CoNLLWriter() {}

    void write(Instance& inst) {
        for (int i = 0; i < inst.size(); ++ i) {
            f << i + 1
                << "\t"
                << inst.forms[i] 
                << "\t"
                << inst.lemmas[i]
                << "\t"
                << inst.postags[i]
                << endl;
        }

        f << endl;
    }
private:
    ofstream& f;
};  // end for ConnllWriter

}   // end for parser
}   // end for namespace ltp


#endif  // end for __CONLL_WRITER_H__
