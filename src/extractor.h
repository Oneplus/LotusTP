#ifndef __EXTRACTOR_H__
#define __EXTRACTOR_H__

#include <iostream>
#include <sstream>
#include <vector>
#include <list>

#include "collections.h"
#include "instance.h"

#include "strvec.hpp"
#include "template.hpp"
#include "strutils.hpp"
#include "strpaste.hpp"

#include "debug.h"

namespace ltp {
namespace parser {
using namespace std;
using namespace ltp::strutils;

class Extractor {
public:
    Extractor() {}
    virtual ~Extractor() {}

    // virtual static int extract1o(Instance * inst, int hid, vector<string> & cache) {}
    // virtual static int extract2o(Instance * inst, int hid, int cid, vector<string> & cache) {}
    // virtual static int extract3o(Instance * inst, int hid, int cid, int gid, vector<string> & cache) {}
protected:
    static void __GET_DIRECTION(int head_id, int child_id, string& direction); 
    static void __GET_DISTANCE_1_2_36_7(int head_id, int child_id, string& distance);
};  //  end for class Extractor


/*
 * POSTag unigram features extractor, this set of features is designed
 * for postagging-dependency parsing joint model.
 *
 * feature template is shown below.
 *
 *  - basic features
 *  -- f(i)
 *  -- f(i-1)
 *  -- f(i+1)
 *
 *  - character features
 *  -- fc(f(i))
 */
class POSUExtractor : public Extractor {
public:
    static int extract1o(Instance * inst, int hid, vector<string> & cache);
    static int extract2o(Instance * inst, int hid, int cid, vector<string> & cache);
    static int extract3o(Instance * inst, int hid, int cid, int sid, vector<string> & cache);
private:
    static const string prefix;
};  //  end for class PosUnigramExtractor


/*
 * POSTag bigram feature extractor
 */
class POSBigramExtractor : public Extractor {
public:
    POSBigramExtractor() : prefix("") { }

    int extract(Instance * inst) {
    }   //  end for int extract
private:
    const string prefix;
};  //  end for class POSBigramExtractor

/*
 * Dependency feature extractor. The feature template is shown as
 * follow, (i is the index of head, j is the index of child), all
 * is combined with direction.
 *
 *  - unigram dependency features
 *  -- form(i) pos(i)
 *  -- form(i)
 *  -- pos(i)
 *  -- form(j) pos(j)
 *  -- form(j)
 *  -- pos(j)
 *
 *  - bigram dependency features
 *  -- form(i) pos(i) form(j) pos(j)
 *  -- pos(i) form(j) pos(j)
 *  -- form(i) form(j) pos(j)
 *  -- form(i) pos(i) pos(j)
 *  -- form(i) pos(i) form(j)
 *  -- form(i) form(j)
 *  -- pos(i) pos(j)
 *
 *  - surrouding dependency features
 *  -- pos(i) pos(i+1) pos(j)
 *  -- pos(i) pos(j-1) pos(j)
 *  -- pos(i) pos(j) pos(j+1)
 *  -- pos(i) pos(i+1) pos(j-1) pos(j)
 *  -- pos(i-1) pos(i+1) pos(j-1) pos(j)
 *  -- pos(i-1) pos(i) pso(j-1) pos(j)
 *  -- pos(i-1) pos(i) pos(i+1) pos(j)
 *
 *  - between dependency features
 *  -- pos(i) pos(b) pos(j) (i<b<j)
 *  -- pos(i) verbcnt(i, j) pos(j)
 *  -- pos(i) conjcnt(i, j) pos(j)
 *  -- pos(i) punccnt(i, j) pos(j)
 */
class DEPExtractor : public Extractor {
public:
    static DEPExtractor * extractor();
    static int num_templates();
    static int extract1o(Instance * inst, int hid, vector<string> & cache);
    static int extract2o(Instance * inst, int hid, int cid, vector< StringVec > & cache);
    static int extract3o(Instance * inst, int hid, int cid, int gid, vector<string> & cache);
protected:
    DEPExtractor();
    ~DEPExtractor();
private:
    static DEPExtractor * instance_;
    static vector<Template *>    templates;
};  //  end for class DependencyExtractor

/*
 * Sibling features extractor. Sibling features is used in 2nd-order parsing model
 * Feature template is shown below (i is head, j is child while s is sibling).
 *
 *  - basic sibling features
 *  -- pos(i) pos(s) pos(j)
 *  -- form(i) pos(s) pos(j)
 *  -- pos(i) form(s) pos(j)
 *  -- pos(i) pos(s) form(j)
 *  -- pos(i) pos(s)
 *  -- form(i) pos(s)
 *  -- pos(i) form(s)
 *  -- form(i) pos(s)
 *  -- pos(j) pos(s)
 *  -- form(j) form(s)
 *  -- form(j) pos(s)
 *  -- pos(j) form(s)
 *
 *  - linear sibling features
 *  -- pos(i) pos(s) pos(s+1)
 *  -- pos(i) pos(s-1) pos(s)
 *  -- pos(i) pos(i+1) pos(s)
 *  -- pos(i-1) pos(i) pos(s)
 *  -- pos(i-1) pos(i) pos(s) pos(s+1)
 *  -- pos(i-1) pos(i) pos(s-1) pos(s)
 *  -- pos(i) pos(i+1) pos(s) pos(s+1)
 *  -- pos(i) pos(i+1) pos(s-1) pos(s)
 *  -- pos(j) pos(s) pos(s+1)
 *  -- pos(j) pos(s-1) pos(s)
 *  -- pos(j) pos(j+1) pos(s)
 *  -- pos(j-1) pos(j) pos(s)
 *  -- pos(j-1) pos(j) pos(s) pos(s+1)
 *  -- pos(j-1) pos(j) pos(s-1) pos(s)
 *  -- pos(j) pos(j+1) pos(s) pos(s+1)
 *  -- pos(j) pos(j+1) pos(s-1) pos(s)
 */
class SIBExtractor : public Extractor {
public:
    static SIBExtractor * extractor();
    static int num_templates();
    static int extract1o(Instance * inst, int hid, vector<string> & cache);
    static int extract2o(Instance * inst, int hid, int cid, vector<string> & cache);
    static int extract3o(Instance * inst, int hid, int cid, int sid, vector< StringVec > & cache);
protected:
    SIBExtractor();
    ~SIBExtractor();
private:
    static SIBExtractor * instance_;
    static vector<Template *> templates;
};  //  end for SiblingExtractor

}   //  end for namespace parser
}   //  end for namespace ltp

#endif  //  end for ifndef __EXTRACTOR_H__
