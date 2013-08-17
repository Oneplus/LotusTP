%module LTP

%{
#include "pyltp.hpp"
%}

%include "std_string.i"
%include "std_vector.i"

namespace std {
    %template(IntVector)        vector<int>;
    %template(DoubleVector)     vector<double>;
    %template(StringVector)     vector<string>;
    %template(ConstCharVector)  vector<const char*>;
};

%include "pyltp.hpp"
