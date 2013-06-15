#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>

#include "options.h"
#include "instance.h"
#include "model.h"
#include "conllreader.h"
#include "conllwriter.h"
#include "extractor.h"
#include "decoder1o.h"

#include "stringmap.hpp"
#include "cfgparser.hpp"
#include "logging.hpp"

#include "debug.h"

namespace ltp {
namespace parser {

using namespace std;
using namespace ltp::utility;

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

        if (__TEST__) {
            // test();
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
    bool parse_cfg(ConfigParser& cfg) {
        string  strbuf;
        int     intbuf;

        model_opt.labeled           = false;
        model_opt.decoder_name      = "1o";
        model_opt.display_interval  = 1000;

        if (cfg.has_section("model")) {
            if (cfg.get_integer("model", "labeled", intbuf)) {
                model_opt.labeled = (intbuf == 1);
            }
            if (cfg.get("model", "decoder-name", strbuf)) {
                model_opt.decoder_name = strbuf;
            } else {
                WARNING_LOG("decoder-name is not configed, [1o] is set as default.");
            }
        }

        train_opt.train_file        = "";
        train_opt.holdout_file      = "";
        train_opt.max_iter          = 10;
        train_opt.algorithm         = "pa";

        if (cfg.has_section("train")) {
            TRACE_LOG("train model specified.");
            __TRAIN__ = true;

            if (cfg.get("train", "train-file", strbuf)) {
                train_opt.train_file = strbuf;
            } else {
                ERROR_LOG("train-file config item is not set.");
                return false;
            }   //  end for if (cfg.get("train", "train-file", strbuf))

            if (cfg.get("train", "holdout-file", strbuf)) {
                train_opt.holdout_file = strbuf;
            } else {
                ERROR_LOG("holdout-file config item is not set.");
                return false;
            }

            if (cfg.get("train", "algorithm", strbuf)) {
                train_opt.algorithm = strbuf;
            } else {
                WARNING_LOG("algorithm is not configed, [PA] is set as default.");
            }

            if (cfg.get_integer("train", "max-iter", intbuf)) {
                train_opt.max_iter = intbuf;
            } else {
                WARNING_LOG("max-iter is not configed, [10] is set as default.");
            }
        }   //  end for cfg.has_section("train")

        feat_opt.use_postag                 =   false;
        feat_opt.use_postag_unigram         =   false;
        feat_opt.use_postag_bigram          =   false;
        feat_opt.use_dependency             =   false;
        feat_opt.use_dependency_unigram     =   false;
        feat_opt.use_dependency_bigram      =   false;
        feat_opt.use_dependency_surrounding =   false;
        feat_opt.use_dependency_between     =   false;
        feat_opt.use_distance_in_dependency_features = true;

        if (cfg.has_section("feature")) {
            if (cfg.get_integer("feature", "use-postag", intbuf)) {
                feat_opt.use_postag = (intbuf == 1);
            }

            if (cfg.get_integer("feature", "use-postag-unigram", intbuf)) {
                feat_opt.use_postag_unigram = (intbuf == 1);
            }

            if (cfg.get_integer("feature", "use-postag-bigram", intbuf)) {
                feat_opt.use_postag_bigram = (intbuf == 1);
            }

            if (cfg.get_integer("feature", "use-dependency", intbuf)) {
                feat_opt.use_dependency = (intbuf == 1);
            }

            if (cfg.get_integer("feature", "use-dependency-unigram", intbuf)) {
                feat_opt.use_dependency_unigram = (intbuf == 1);
            }

            if (cfg.get_integer("feature", "use-dependency-bigram", intbuf)) {
                feat_opt.use_dependency_bigram = (intbuf == 1);
            }

            if (cfg.get_integer("feature", "use-dependency-surrounding", intbuf)) {
                feat_opt.use_dependency_surrounding = (intbuf == 1);
            }

            if (cfg.get_integer("feature", "use-dependency-between", intbuf)) {
                feat_opt.use_dependency_between = (intbuf == 1);
            }


            feat_opt.use_unlabeled_dependency = true;
            if (model_opt.labeled) {
            }
        }
    }

    void display_opt() {
        cerr << "Train-File : "    << train_opt.train_file     << "\n"
            << "Holdout-File : "   << train_opt.holdout_file   << "\n"
            << "Max-Iteration : "  << train_opt.max_iter       << endl;
    }

    bool read_data(const char * filename, 
            vector<Instance *>& dat) {
        Instance * inst = NULL;
        ifstream f(filename);
        if (!f) {
            return false;
        }

        CoNLLReader reader(f);

        int num_inst = 0;
        while ((inst = reader.next())) {
            dat.push_back(inst);
            ++ num_inst;

            if (num_inst % model_opt.display_interval == 0) {
                TRACE_LOG("Reading in [%d] instances.", num_inst);
            }
        }
        return true;
    }

    int build_extractors(bool train) {
        int num_extractors = 0;
        if (feat_opt.use_postag_unigram) {
            POSUnigramExtractor * extractor = new POSUnigramExtractor(model->collections, train);
            extractors.push_back( dynamic_cast<Extractor *>(extractor) );
            TRACE_LOG("[Postag Unigram Feature] extractor is configed.");
            num_extractors ++;
        }

        if (feat_opt.use_dependency) {
            DependencyExtractor * extractor = new DependencyExtractor(model->collections, train);
            extractors.push_back( dynamic_cast<Extractor *>(extractor) );
            TRACE_LOG("[Dependency Feature] extractor is configed.");
            num_extractors ++;
        }

        return num_extractors;
    }

    int execute_extraction(const vector<Instance *>& dat) {
        for (int i = 0; i < dat.size(); ++ i) {
            for (int j = 0; j < extractors.size(); ++ j) {
                extractors[j]->extract(dat[i]);
            }

            if ((i + 1) % model_opt.display_interval == 0) {
                TRACE_LOG("[%d] instances is extracted.", i + 1);
            }
        }
        return 0;
    }

    void build_features_of_one_instance(Instance * inst, const vector<int> &heads, SparseVec& vec) {
        vec.zero();
        for (int i = 1; i < inst->size(); ++ i) {
            int hid = heads[i];
            if (feat_opt.use_dependency) {
                vec.add(inst->dependency_features[hid][i]);
            }
        }
    }

    void turn_off_all_extractors() {
        for (int i = 0; i < extractors.size(); ++ i) {
            extractors[i]->off();
        }
    }

    void build_gold_features() {
        for (int i = 0; i < train_dat.size(); ++ i) {
            build_features_of_one_instance(train_dat[i], 
                    train_dat[i]->heads, 
                    train_dat[i]->features);
        }
    }

    void train() {
        const char * train_file = train_opt.train_file.c_str();
        const char * holdout_file = train_opt.holdout_file.c_str();

        if (!read_data(train_file, train_dat)) {
            ERROR_LOG("Failed to read train data from [%s].", train_file);
            return;
        } else {
            TRACE_LOG("Read in [%d] train instances.", train_dat.size());
        }

        if (!read_data(train_opt.holdout_file.c_str(), holdout_dat)) {
            ERROR_LOG("Failed to read holdout data from [%s].", holdout_file);
            return;
        } else {
            TRACE_LOG("Read in [%d] holdout instances.", holdout_dat.size());
        }

        model = new Model;

        build_extractors(true);

        TRACE_LOG("Start extracting features.");
        execute_extraction(train_dat);
        turn_off_all_extractors();
        execute_extraction(holdout_dat);

        TRACE_LOG("Extracting feature is done.");
        TRACE_LOG("Number of features: [%d]", model->collections.dim());

        build_gold_features();
#if DEBUG
        model->collections.dump(cout);
#endif  //  end for DEBUG
        model->param.realloc(model->collections.dim());
        decoder = new Decoder1O();
        // model->collections.dump(cout);

        for (int iter = 0; iter < train_opt.max_iter; ++ iter) {
            TRACE_LOG("Start training epoch #%d.", (iter + 1));
            for (int i = 0; i < train_dat.size(); ++ i) {
                calculate_score(train_dat[i], model->param);
                decoder->decode(train_dat[i]);

                build_features_of_one_instance(train_dat[i],
                        train_dat[i]->predicted_heads,
                        train_dat[i]->predicted_features);

#if DEBUG
                instance_verify(train_dat[i], cout, true);
#endif  //  end for DEBUG

                if (train_opt.algorithm == "pa") {
                    SparseVec update_features;

                    update_features.add(train_dat[i]->features, 1.);
                    update_features.add(train_dat[i]->predicted_features, -1.);

                    double error = errors(train_dat[i]);
                    double score = model->param.dot(update_features, false);
                    double norm = update_features.L2();
                    double step = 0.;

                    if (norm < 1e-4 && norm > -1e-4) {
                        step = 0;
                    } else {
                        step = (error - score) / norm;
                    }

                    model->param.add(update_features, 
                            iter * train_dat.size() + i + 1, 
                            step);
                } else if (train_opt.algorithm == "ap") {
                    SparseVec update_features;

                    update_features.add(train_dat[i]->features, 1.);
                    update_features.add(train_dat[i]->predicted_features, -1.);

                    model->param.add(update_features,
                            iter * train_dat.size() + i + 1,
                            1.);
                }

#if DEBUG
                 model->param.str(cout);
#endif  //  end for DEBUG
                // instance_verify(train_dat[i], cout, false);

                if ((i + 1) % model_opt.display_interval == 0) {
                    TRACE_LOG("[%d] instances is trained.", i + 1);
                }
            }

            // model->param.divide( train_dat.size() * (iter + 1) );

            int head_correct = 0;
            int label_correct = 0;
            int total_rels = 0;

            for (int i = 0; i < holdout_dat.size(); ++ i) {
                calculate_score(holdout_dat[i], model->param);
                decoder->decode(holdout_dat[i]);

#if DEBUG
                build_features_of_one_instance(holdout_dat[i],
                        holdout_dat[i]->predicted_heads,
                        holdout_dat[i]->predicted_features);
                instance_verify(holdout_dat[i], cout, true);
#endif //   end for DEBUG
                total_rels += (holdout_dat[i]->size() - 1);
                head_correct += (holdout_dat[i]->size() - 1 - errors(holdout_dat[i], false));
            }

            TRACE_LOG("UAS: %.4lf ( %d / %d )", (double)head_correct / total_rels, head_correct, total_rels);
        }
    }

    int errors(const Instance * inst, bool ignore_puncation = true) {
        int len = inst->size();
        int ret = 0;
        for (int j = 1; j  < len; ++ j) {
            if (ignore_puncation && inst->postags[j] == "wp") {
                continue;
            }

            if (inst->predicted_heads[j] != inst->heads[j]) {
                ++ ret;
            }
        }
        return ret;
    }

    void calculate_score(Instance * inst, const Parameters& param) {
        int len = inst->size();
        if (feat_opt.use_postag_unigram) {
            for (int i = 0; i < len; ++ i) {
                inst->postag_unigram_scores[i] = 0.;
                FeatureVector * fv = inst->postag_unigram_features[i];
                for (int j = 0; j < fv->n; ++ j) {
                    inst->postag_unigram_scores[i] += param[(fv->idx[j])];
                }
            }
        }   //  end if feat_opt.use_postag_unigram

        if (feat_opt.use_dependency) {
            for (int i = 0; i < len; ++ i) {
                for (int j = 1; j < len; ++ j) {
                    FeatureVector * fv = inst->dependency_features[i][j];
                    inst->dependency_scores[i][j] = 0.;
                    if (!fv) {
                        continue;
                    }
                    inst->dependency_scores[i][j] = param.dot(fv);
                }
            }
        }   //  end if feat_opt.use_dependency
    }
};  //  end for class Parser
}   //  end for namespace parser
}   //  end for namespace ltp

#endif  // end for __PARSER_H__
