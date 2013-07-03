#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <iostream>

namespace ltp {
namespace parser {

using namespace std;

const string ROOT_FORM      =   "RT#";
const string ROOT_LEMMA     =   "RT#";
const string ROOT_CPOSTAG   =   "RT#";
const string ROOT_POSTAG    =   "RT#";
const string ROOT_FEAT      =   "RT#";
const string ROOT_DEPREL    =   "RT#";

const string PRP            =   "PRP";
const string PRP2           =   "PRP2";

const string OOV            =   "-OOV-";
const string NONE_FORM      =   "##";
const string NONE_LEMMA     =   "##";
const string NONE_CPOSTAG   =   "##";
const string NONE_POSTAG    =   "##";
const string NONE_FEAT      =   "##";

const string FSEP           =   "-";

const double DOUBLE_POS_INF =   1e20;
const double DOUBLE_NEG_INF =   -1e20;
const double EPS            =   1e-10;

const size_t CMP            =   0;
const size_t INCMP          =   1;

enum {
    DEPU,       //  Unlabeled Standard Features
    DEPL,       //  Labeled Standard Features
    SIBU,       //  Unlabeled Sibling Features
    SIBL,       //  Labeled Sibling Features
    GRDU,       //  Unlabeled Grandchild Feature
    GRDL,       //  Labeled Grandchild Feature
    GRDSIBU,    //  Unlabeled Grand Sibling Features
    GRDSIBL,
    POSU,
    POSB,
};

}       //  end for namespace parser
}       //  end for namespace ltp
#endif  //  end for __SETTINGS_H__

