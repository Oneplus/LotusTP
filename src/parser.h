#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>

#include "instance.h"
#include "model.h"
#include "conllreader.h"
#include "conllwriter.h"
#include "extractor.h"
#include "decoder.h"

#include "cfgparser.hpp"
#include "logging.hpp"
#include "time.hpp"

#include "debug.h"

using namespace std;
using namespace ltp::utility;
using namespace ltp::strutils;

namespace ltp {
namespace parser {

class Parser{

/* Parser Options */
private:
    bool            __TRAIN__;
    bool            __TEST__;

public:
    Parser( ConfigParser& cfg ) {
        parse_cfg(cfg);
        // display_opt();
    }

    ~Parser() {
    }

    bool operator! () const {
        return _valid;
    }

    void run() {
        /* running train process */
        if (__TRAIN__) {
            train();
        }

        /* running test process */
        if (__TEST__) {
            test();
        }
    }

private:
    bool _valid; /* indicating if the parser is valid */
    Model * model;
    vector<Instance *> train_dat;
    vector<Instance *> holdout_dat;
    vector<Extractor *> extractors;
    Decoder * decoder;
private:
    bool parse_cfg(ConfigParser& cfg);
    void display_opt();

    bool read_instances(const char * filename, vector<Instance *>& dat);

    void build_feature_space(void);

    void build_configuration(void);

    void collect_unlabeled_features_of_one_instance(Instance * inst,
            const vector<int> & heads,
            SparseVec & vec);

    void collect_labeled_features_of_one_instance(Instance * inst,
            const vector<int> & heads,
            const vector<int> & deprelsidx,
            SparseVec & vec);

    void collect_features_of_one_instance(Instance * inst, 
            bool gold = false);

    void extract_features(Instance * inst);

    void extract_features(vector<Instance *>& dat) {
        // ofstream out("cdt.retrieve.dat", std::ostream::out);
        // DependencyExtractor extractor;
        for (int i = 0; i < dat.size(); ++ i) {
            extract_features(dat[i]);
            if ((i + 1) % model_opt.display_interval == 0) {
                TRACE_LOG("[%d] instance is extracted.", i + 1);
            }
        }   // end for i = 0; i < dat.size(); ++ i
    }

    void build_gold_features() {
        for (int i = 0; i < train_dat.size(); ++ i) {
            collect_features_of_one_instance(train_dat[i], true);
        }
    }

    void train(void);

    void evaluate(void);

    void test(void);

    void calculate_score(Instance * inst, const Parameters& param, bool use_avg = false);

};  //  end for class Parser
}   //  end for namespace parser
}   //  end for namespace ltp

#endif  // end for __PARSER_H__
