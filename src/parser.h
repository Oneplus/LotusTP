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

#include "cfgparser.hpp"
#include "logging.hpp"
#include "time.hpp"

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

        __TRAIN__ = false;

        train_opt.train_file        = "";
        train_opt.holdout_file      = "";
        train_opt.max_iter          = 10;
        train_opt.algorithm         = "pa";
        train_opt.model_name        = "";

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

            train_opt.model_name = train_opt.train_file + "." + train_opt.algorithm + ".model";
            if (cfg.get("train", "model-name", strbuf)) {
                train_opt.model_name = strbuf;
            } else {
                WARNING_LOG("model name is not configed, [%s] is set as default",
                        train_opt.model_name.c_str());
            }

            if (cfg.get_integer("train", "max-iter", intbuf)) {
                train_opt.max_iter = intbuf;
            } else {
                WARNING_LOG("max-iter is not configed, [10] is set as default.");
            }
        }   //  end for cfg.has_section("train")

        __TEST__ = false;

        test_opt.test_file  = "";
        test_opt.model_file = "";

        if (cfg.has_section("test")) {
            __TEST__ = true;

            if (cfg.get("test", "test-file", strbuf)) {
                test_opt.test_file = strbuf;
            } else {
                ERROR_LOG("test-file config item is not set.");
                return false;
            }   //  end for if (cfg.get("train", "train-file", strbuf))

            if (cfg.get("test", "model-file", strbuf)) {
                test_opt.model_file = strbuf;
            } else {
                ERROR_LOG("model-file config item is not set.");
                return false;
            }
        }

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

    bool read_instances(const char * filename, 
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

    void build_feature_space(void) {
        // ofstream out("cdt.build.dat", std::ostream::out);
        if (feat_opt.use_dependency) {
            DependencyExtractor extractor;
            Dictionary * dict = model->collections.create_dict("dependency");

            for (int i = 0; i < train_dat.size(); ++ i) {
                for (int j = 1; j < train_dat[i]->size(); ++ j) {
                    int hid = train_dat[i]->heads[j];
                    int cid = j;

                    vector<string> cache;
                    extractor.extract2o(train_dat[i],
                            hid,
                            cid,
                            cache);

                    for (int k = 0; k < cache.size(); ++ k) {
                        // out << cache[k] << endl;
                        dict->retrieve(cache[k].c_str(), true);
                    }
                }
            }
        }
        // out.close();
    }   //  end for build_feature_space

    void build_features_of_one_instance(Instance * inst, const vector<int> &heads, SparseVec& vec) {
        vec.zero();
        for (int i = 1; i < inst->size(); ++ i) {
            int hid = heads[i];
            if (feat_opt.use_dependency) {
                vec.add(inst->dependency_features[hid][i], 1.);
            }
        }
    }

    void extract_features(Instance * inst) {
        int len = inst->size();
        if (feat_opt.use_dependency) {
            Dictionary * dict = model->collections.create_dict("dependency");

            inst->dependency_features.resize(len, len);
            inst->dependency_scores.resize(len, len);

            vector<string>  cache;
            vector<int>     cache_again;

            cache.reserve(100);

            for (int hid = 0; hid < len; ++ hid) {
                for (int cid = 1; cid < len; ++ cid) {
                    inst->dependency_features[hid][cid] = NULL;
                    inst->dependency_scores[hid][cid] = 0.;

                    if (hid == cid) {
                        continue;
                    }

                    cache.clear();
                    cache_again.clear();

                    DependencyExtractor::extract2o(inst,
                            hid,
                            cid,
                            cache);

                    for (int k = 0; k < cache.size(); ++ k) {
                        // out << cache[k] << endl;
                       int idx = dict->retrieve(cache[k].c_str(), false);
                       if (idx >= 0) {
                           cache_again.push_back(idx);
                       }
                    }

                    int num_feat = cache_again.size();

                    inst->dependency_features[hid][cid] = new FeatureVector;
                    inst->dependency_features[hid][cid]->n = num_feat;
                    inst->dependency_features[hid][cid]->idx = 0;
                    inst->dependency_features[hid][cid]->val = 0;

                    if (num_feat > 0) {
                        inst->dependency_features[hid][cid]->idx = new int[num_feat];
                        for (int j = 0; j < num_feat; ++ j) {
                            inst->dependency_features[hid][cid]->idx[j] = cache_again[j];
                        }
                    }
                }   //  end for for cid = 1; cid < len; ++ cid
            }   //  end for hid = 0; hid < len; ++ hid
        }
    }

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
            build_features_of_one_instance(train_dat[i], 
                    train_dat[i]->heads, 
                    train_dat[i]->features);
        }
    }

    void train() {
        const char * train_file = train_opt.train_file.c_str();
        const char * holdout_file = train_opt.holdout_file.c_str();

        if (!read_instances(train_file, train_dat)) {
            ERROR_LOG("Failed to read train data from [%s].", train_file);
            return;
        } else {
            TRACE_LOG("Read in [%d] train instances.", train_dat.size());
        }

        if (!read_instances(train_opt.holdout_file.c_str(), holdout_dat)) {
            ERROR_LOG("Failed to read holdout data from [%s].", holdout_file);
            return;
        } else {
            TRACE_LOG("Read in [%d] holdout instances.", holdout_dat.size());
        }

        model = new Model;

        TRACE_LOG("Start building feature space.");
        build_feature_space();
        TRACE_LOG("Building feature space is done.");
        TRACE_LOG("Number of features: [%d]", model->collections.dim());

        TRACE_LOG("Start extracting features.");
        extract_features(train_dat);
        extract_features(holdout_dat);
        TRACE_LOG("Extracting feature is done.");

        build_gold_features();
#if DEBUG
        model->collections.dump(cout);
#endif  //  end for DEBUG
        model->param.realloc(model->collections.dim());
        decoder = new Decoder1O();
        // model->collections.dump(cout);

        for (int iter = 0; iter < train_opt.max_iter; ++ iter) {
            TRACE_LOG("Start training epoch #%d.", (iter + 1));

            // random_shuffle(train_dat.begin(), train_dat.end());
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
                    update_features.zero();
                    /*for (int j = 1; j < train_dat[i]->size(); ++ j) {
                        if (train_dat[i]->heads[j] != train_dat[i]->predicted_heads[j]) {
                            int hid, cid;
                            hid = train_dat[i]->heads[j];
                            cid = j;
                            update_features.add(train_dat[i]->dependency_features[hid][cid], 1.);

                            hid = train_dat[i]->predicted_heads[j];
                            cid = j;
                            update_features.add(train_dat[i]->dependency_features[hid][cid], -1.);
                        }
                    }*/
                    update_features.add(train_dat[i]->features, 1.);
                    update_features.add(train_dat[i]->predicted_features, -1.);

                    double error = train_dat[i]->num_errors(false);
                    double score = model->param.dot(update_features, false);
                    double norm = update_features.L2();
                    double step = 0.;

                    if (norm < EPS) {
                        step = 0;
                    } else {
                        step = (error - score) / norm;
                    }

                    // update_features.str(cout);
                    // cout << "parser::train - num error = " << error << endl;
                    // cout << "parser::train - norm = " << norm << endl;
                    // cout << "parser::train - step = " << step << endl;
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

            model->param.flush( train_dat.size() * (iter + 1) );

            ofstream fout((train_opt.model_name + "." + to_str(iter) + ".model").c_str(),
                    std::ofstream::binary);
            model->save(fout);

            int head_correct = 0;
            int label_correct = 0;
            int total_rels = 0;

            for (int i = 0; i < holdout_dat.size(); ++ i) {
                calculate_score(holdout_dat[i], model->param, true);
                decoder->decode(holdout_dat[i]);

#if DEBUG
                build_features_of_one_instance(holdout_dat[i],
                        holdout_dat[i]->predicted_heads,
                        holdout_dat[i]->predicted_features);
                instance_verify(holdout_dat[i], cout, true);
#endif //   end for DEBUG

                int num_rels = holdout_dat[i]->num_rels();
                total_rels += num_rels;
                head_correct += (num_rels - holdout_dat[i]->num_errors());
            }

            TRACE_LOG("UAS: %.4lf ( %d / %d )", (double)head_correct / total_rels, head_correct, total_rels);
        }
    }

    void test() {
        const char * model_file = test_opt.model_file.c_str();
        ifstream mfs(model_file, std::ifstream::binary);

        if (!mfs) {
            return;
        }

        model = new Model;
        if (!model->load(mfs)) {
            return;
        }

        const char * test_file = test_opt.test_file.c_str();

        ifstream f(test_file);
        if (!f) {
            return;
        }

        CoNLLReader reader(f);

        Instance * inst = NULL;

        decoder = new Decoder1O();

        double before = get_time();
        while ((inst = reader.next())) {
            extract_features(inst);
            calculate_score(inst, model->param);

            decoder->decode(inst);

            instance_verify(inst, cout, false);
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

        if (feat_opt.use_dependency) {
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
        }   //  end if feat_opt.use_dependency
    }
};  //  end for class Parser
}   //  end for namespace parser
}   //  end for namespace ltp

#endif  // end for __PARSER_H__
