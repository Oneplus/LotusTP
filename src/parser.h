#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>

#include "instance.h"
#include "model.h"
#include "conllreader.h"
#include "conllwriter.h"
#include "extractor.h"
#include "decoder1o.h"

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

    void collect_unlabeled_dependency_features_of_one_instance(Instance * inst,
            const vector<int> & heads,
            SparseVec & vec);

    void collect_labeled_dependency_features_of_one_instance(Instance * inst,
            const vector<int> & heads,
            const vector<int> & deprelsidx,
            SparseVec & vec);

    void collect_features_of_one_instance(Instance * inst, 
            bool gold = false);

    void build_unlabeled_features_of_one_instance(Instance * inst,
            const vector<int> &heads,
            SparseVec & vec) {
        vec.zero();
        for (int i = 1; i < inst->size(); ++ i) {
            int hid = heads[i];
            vec.add(inst->dependency_features[hid][i], 1.);
        }
    }

    void build_labeled_features_of_one_instance(Instance * inst, 
            const vector<int> &heads,
            const vector<int> &deprelsidx,
            SparseVec & vec) {

        vec.zero();
        for (int i = 1; i < inst->size(); ++ i) {
            int hid = heads[i];
            int relidx = deprelsidx[i];

            vec.add(inst->labeled_dependency_features[hid][i][relidx], 1.);
        }
    }

    void build_features_of_one_instance(Instance * inst, bool train = false) {
        if (train && feat_opt.use_unlabeled_dependency) {
            build_unlabeled_features_of_one_instance(inst,
                    inst->heads,
                    inst->features);
        } else if (train && feat_opt.use_labeled_dependency) {
            build_labeled_features_of_one_instance(inst,
                    inst->heads,
                    inst->deprelsidx,
                    inst->features);
        } else if (!train && feat_opt.use_unlabeled_dependency) {
            build_unlabeled_features_of_one_instance(inst,
                    inst->predicted_heads,
                    inst->predicted_features);
        } else if (!train && feat_opt.use_labeled_dependency) {
            build_labeled_features_of_one_instance(inst,
                    inst->predicted_heads,
                    inst->predicted_deprelsidx,
                    inst->predicted_features);
        }
    }

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
            build_features_of_one_instance(train_dat[i], true);
        }
    }

    void train(void);

    void evaluate(void);

    void test() {
        double before = get_time();
        const char * model_file = test_opt.model_file.c_str();
        ifstream mfs(model_file, std::ifstream::binary);

        if (!mfs) {
            return;
        }

        model = new Model;
        if (!model->load(mfs)) {
            return;
        }

        TRACE_LOG("Number of deprels [%d]", model->num_deprels());
        TRACE_LOG("Number of dimension [%d]", model->dim());
        TRACE_LOG("Number of features [%d]", model->num_features());
        TRACE_LOG("Labeled: %s", (model_opt.labeled ? "true" : "fales"));
        TRACE_LOG("Decoder: %s", model_opt.decoder_name.c_str());
        TRACE_LOG("Dependency features unigram:     %s", 
                (feat_opt.use_dependency_unigram ? "true" : "false"));
        TRACE_LOG("Dependency features bigram:      %s", 
                (feat_opt.use_dependency_bigram ? "true" : "false"));
        TRACE_LOG("Dependency features surrounding: %s", 
                (feat_opt.use_dependency_surrounding ? "true" : "false"));
        TRACE_LOG("Dependency features between:     %s", 
                (feat_opt.use_dependency_between ? "true" : "false"));

        const char * test_file = test_opt.test_file.c_str();

        ifstream f(test_file);
        if (!f) {
            return;
        }

        CoNLLReader reader(f);

        Instance * inst = NULL;

        if (!model_opt.labeled) {
            decoder = new Decoder1O();
        } else {
            decoder = new Decoder1O(model->num_deprels());
        }

        int head_correct = 0;
        int label_correct = 0;
        int total_rels = 0;

        cerr << get_time() - before << endl;
        before = get_time();
        while ((inst = reader.next())) {

            if (model_opt.labeled) {
                inst->deprelsidx.resize(inst->size());
                for (int i = 1; i < inst->size(); ++ i) {
                    inst->deprelsidx[i] = model->deprels.index(inst->deprels[i].c_str());
                }
            }

            extract_features(inst);
            calculate_score(inst, model->param);

            decoder->decode(inst);

            instance_verify(inst, cout, true);

            total_rels += inst->num_rels();
            head_correct += inst->num_correct_heads();
            label_correct += inst->num_correct_heads_and_labels();

            delete inst;
        }

        TRACE_LOG("UAS: %.4lf ( %d / %d )", (double)head_correct / total_rels, head_correct, total_rels);

        if (model_opt.labeled) {
            TRACE_LOG("LAS: %.4lf ( %d / %d )", 
                    (double)label_correct / total_rels, 
                    label_correct, 
                    total_rels);
        }

        double after = get_time();
        cerr << after - before << endl;
        sleep(1000000);
    }

    void calculate_score(Instance * inst, const Parameters& param, bool use_avg = false) {
        int len = inst->size();
        if (feat_opt.use_postag_unigram) {
            for (int i = 0; i < len; ++ i) {
                inst->postag_unigram_scores[i] = 0.;
                FeatureVector * fv = inst->postag_unigram_features[i];
                inst->postag_unigram_scores[i] = param.dot(fv, false);
            }
        }   //  end if feat_opt.use_postag_unigram

        if (feat_opt.use_unlabeled_dependency) {
            for (int i = 0; i < len; ++ i) {
                for (int j = 1; j < len; ++ j) {
                    FeatureVector * fv = inst->dependency_features[i][j];
                    inst->dependency_scores[i][j] = DOUBLE_NEG_INF;

                    if (!fv) {
                        continue;
                    }
                    inst->dependency_scores[i][j] = param.dot(fv, use_avg);
                }
            }
        }   //  end if feat_opt.use_unlabeled_dependency

        if (feat_opt.use_labeled_dependency) {
            for (int i = 0; i < len; ++ i) {
                for (int j = 1; j < len; ++ j) {
                    for (int l = 0; l < model->num_deprels(); ++ l) {
                        FeatureVector * fv = inst->labeled_dependency_features[i][j][l];
                        inst->labeled_dependency_scores[i][j][l] = DOUBLE_NEG_INF;

                        if (!fv) {
                            continue;
                        }

                        inst->labeled_dependency_scores[i][j][l] = param.dot(fv, use_avg);
                    }
                }
            }
        }   //  end if feat_opt.use_labeled_dependency
    }
};  //  end for class Parser
}   //  end for namespace parser
}   //  end for namespace ltp

#endif  // end for __PARSER_H__
