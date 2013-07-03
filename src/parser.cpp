#include "parser.h"
#include "options.h"

namespace ltp {
namespace parser {

// display option
void Parser::display_opt() {
    cerr << "Train-File : "    << train_opt.train_file     << "\n"
        << "Holdout-File : "   << train_opt.holdout_file   << "\n"
        << "Max-Iteration : "  << train_opt.max_iter       << endl;
}

bool Parser::parse_cfg(utility::ConfigParser & cfg) {
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
    // feat_opt.use_distance_in_dependency_features = false;

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

        // detrieve dependency type from configuration
        feat_opt.use_unlabeled_dependency = (model_opt.labeled == false &&
                feat_opt.use_dependency);

        feat_opt.use_labeled_dependency = (model_opt.labeled == true &&
                feat_opt.use_dependency);
    }
}

void Parser::build_configuration(void) {
    // build postags set, deprels set.
    // map deprels from string to int when model_opt.labeled is configed.
    // need to check if the model is initialized

    for (int i = 0; i < train_dat.size(); ++ i) {
        int len = train_dat[i]->size();

        // if labeled is configured, init the deprelsidxs
        if (model_opt.labeled) {
            train_dat[i]->deprelsidx.resize(len);
            train_dat[i]->predicted_deprelsidx.resize(len);
        }

        for (int j = 0; j < len; ++ j) {
            model->postags.push(train_dat[i]->postags[j].c_str());
            if (model_opt.labeled) {
                int idx = -1;
                idx = model->deprels.push(train_dat[i]->deprels[j].c_str());
                train_dat[i]->deprelsidx[j] = idx;
            }
        }
    }
}

void Parser::build_feature_space(void) {
    model->space.build_feature_space(model->num_deprels(), train_dat);
}   //  end for build_feature_space

void Parser::collect_unlabeled_dependency_features_of_one_instance(Instance * inst,
        const vector<int> & heads,
        SparseVec & vec ) {
    for (int i = 1; i < inst->size(); ++ i) {
        int hid = heads[i];
        vec.add(inst->dependency_features[hid][i], 1.);
    }
}

void Parser::collect_labeled_dependency_features_of_one_instance(Instance * inst,
        const vector<int> & heads,
        const vector<int> & deprelsidx,
        SparseVec & vec ) {
    for (int i = 1; i < inst->size(); ++ i) {
        int hid = heads[i];
        int relidx= deprelsidx[i];
        vec.add(inst->labeled_dependency_features[hid][i][relidx], 1.);
    }
}

void Parser::collect_features_of_one_instance(Instance * inst, bool gold) {
    if (gold && feat_opt.use_unlabeled_dependency) {
        collect_unlabeled_dependency_features_of_one_instance(inst,
                inst->heads,
                inst->features);
    } else if (gold && feat_opt.use_labeled_dependency) {
        collect_labeled_dependency_features_of_one_instance(inst,
                inst->heads,
                inst->deprelsidx,
                inst->features);
    } else if (!gold && feat_opt.use_unlabeled_dependency) {
        collect_unlabeled_dependency_features_of_one_instance(inst,
                inst->predicted_heads,
                inst->predicted_features);
    } else if (!gold && feat_opt.use_labeled_dependency) {
        collect_labeled_dependency_features_of_one_instance(inst,
                inst->predicted_heads,
                inst->predicted_deprelsidx,
                inst->predicted_features);
    }
}

bool Parser::read_instances(const char * filename, vector<Instance *> & dat) {
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

void Parser::extract_features(Instance * inst) {
    int len = inst->size();
    FeatureSpace& space = model->space;

    if (feat_opt.use_dependency) {

        if (!model_opt.labeled) {
            inst->dependency_features.resize(len, len);
            inst->dependency_scores.resize(len, len);
        } else {
            inst->labeled_dependency_features.resize(len, len, model->num_deprels());
            inst->labeled_dependency_scores.resize(len, len, model->num_deprels());
        }

        vector< StringVec >  cache;
        vector< int >     cache_again;

        int N = DEPExtractor::num_templates();

        cache.resize( N );

        for (int hid = 0; hid < len; ++ hid) {
            for (int cid = 0; cid < len; ++ cid) {

                if (!model_opt.labeled) {
                    inst->dependency_features[hid][cid] = NULL;
                    inst->dependency_scores[hid][cid] = 0.;
                } else {
                    for (int l = 0; l < model->num_deprels(); ++ l) {
                        inst->labeled_dependency_features[hid][cid][l] = NULL;
                        inst->labeled_dependency_scores[hid][cid][l] = 0.;
                    }
                }

                if (hid == cid) {
                    continue;
                }

                // here the self-implementated String Vector is little
                // fasteer than the list<string>
                for (int i = 0; i < N; ++ i) {
                    cache[i].clear();
                }

                DEPExtractor::extract2o(inst,
                        hid,
                        cid,
                        cache);

                cache_again.clear();

                for (int tid = 0; tid < cache.size(); ++ tid) {
                    for (int itx = 0; itx < cache[tid].size(); ++ itx) {
                        int idx = model->space.index(FeatureSpace::DEP, tid, cache[tid][itx], 0);
                        // std::cout << "idx: " << idx << std::endl;
                        if (idx >= 0) {
                            cache_again.push_back(idx);
                        }
                    }
                }

                int num_feat = cache_again.size();

                if (!model_opt.labeled) {

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
                } else {
                    for (int l = 0; l < model->num_deprels(); ++ l) {
                        inst->labeled_dependency_features[hid][cid][l] = new FeatureVector;
                        inst->labeled_dependency_features[hid][cid][l]->n = num_feat;
                        inst->labeled_dependency_features[hid][cid][l]->idx = 0;
                        inst->labeled_dependency_features[hid][cid][l]->val = 0;

                        if (num_feat > 0) {
                            inst->labeled_dependency_features[hid][cid][l]->idx = new int[num_feat];
                            for (int j = 0; j < num_feat; ++ j) {
                                inst->labeled_dependency_features[hid][cid][l]->idx[j] = (
                                        cache_again[j] + l);
                            }
                        }
                    }
                }
            }   //  end for for cid = 1; cid < len; ++ cid
        }   //  end for hid = 0; hid < len; ++ hid
    }
}

void Parser::train(void) {
    const char * train_file = train_opt.train_file.c_str();
    const char * holdout_file = train_opt.holdout_file.c_str();

    if (!read_instances(train_file, train_dat)) {
        ERROR_LOG("Failed to read train data from [%s].", train_file);
        return;
    } else {
        TRACE_LOG("Read in [%d] train instances.", train_dat.size());
    }

    model = new Model;

    TRACE_LOG("Start building configuration.");
    build_configuration();
    TRACE_LOG("Building configuration is done.");
    TRACE_LOG("Number of postags: [%d]", model->num_postags());
    TRACE_LOG("Number of deprels: [%d]", model->num_deprels());

    TRACE_LOG("Start building feature space.");
    build_feature_space();
    TRACE_LOG("Building feature space is done.");
    TRACE_LOG("Number of features: [%d]", model->space.num_features());

    TRACE_LOG("Start extracting features.");
    extract_features(train_dat);
    TRACE_LOG("Extracting feature is done.");

    build_gold_features();

    model->param.realloc(model->dim());
    TRACE_LOG("Allocate a parameter vector of [%d] dimension.", model->dim());

    if (!model_opt.labeled) {
        decoder = new Decoder1O();
        TRACE_LOG("1st-Order Decoder without label is configured.");
    } else {
        decoder = new Decoder1O(model->num_deprels());
        TRACE_LOG("1st-Order Decoder with [%d] labels is configured.", model->num_deprels());
    }

    for (int iter = 0; iter < train_opt.max_iter; ++ iter) {
        TRACE_LOG("Start training epoch #%d.", (iter + 1));

        // random_shuffle(train_dat.begin(), train_dat.end());
        for (int i = 0; i < train_dat.size(); ++ i) {
            calculate_score(train_dat[i], model->param);
            decoder->decode(train_dat[i]);

            build_features_of_one_instance(train_dat[i], false);

            // instance_verify(train_dat[i], cout, true);

            if (train_opt.algorithm == "pa") {
                SparseVec update_features;
                update_features.zero();
                update_features.add(train_dat[i]->features, 1.);
                update_features.add(train_dat[i]->predicted_features, -1.);

                double error = train_dat[i]->num_errors();
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

            if ((i + 1) % model_opt.display_interval == 0) {
                TRACE_LOG("[%d] instances is trained.", i + 1);
            }
        }

        model->param.flush( train_dat.size() * (iter + 1) );
        evaluate();

        ofstream fout((train_opt.model_name + "." + to_str(iter) + ".model").c_str(),
                std::ofstream::binary);
        model->save(fout);

    }
}

void Parser::evaluate(void) {
    const char * holdout_file = train_opt.holdout_file.c_str();

    int head_correct = 0;
    int label_correct = 0;
    int total_rels = 0;

    if (!read_instances(holdout_file, holdout_dat)) {
        ERROR_LOG("Failed to read holdout data from [%s].", holdout_file);
        return;
    } else {
        TRACE_LOG("Read in [%d] holdout instances.", holdout_dat.size());
    }

    for (int i = 0; i < holdout_dat.size(); ++ i) {
        // build up labeled identification
        int len = holdout_dat[i]->size();

        if (model_opt.labeled) {
            holdout_dat[i]->deprelsidx.resize(len, -1);
            holdout_dat[i]->predicted_deprelsidx.resize(len, -1);

            for (int j = 0; j < len; ++ j) {

                int idx = -1;
                idx = model->deprels.index(holdout_dat[i]->deprels[j].c_str());
                holdout_dat[i]->deprelsidx[j] = idx;
            }
        }

        extract_features(holdout_dat[i]);
        calculate_score(holdout_dat[i], model->param, true);
        decoder->decode(holdout_dat[i]);

        // instance_verify(holdout_dat[i], cout, true);
#if DEBUG
        build_features_of_one_instance(holdout_dat[i], false);
        instance_verify(holdout_dat[i], cout, true);
#endif //   end for DEBUG

        total_rels += holdout_dat[i]->num_rels();
        head_correct += holdout_dat[i]->num_correct_heads();
        label_correct += holdout_dat[i]->num_correct_heads_and_labels();

        holdout_dat[i];
    }

    TRACE_LOG("UAS: %.4lf ( %d / %d )", 
            (double)head_correct / total_rels, 
            head_correct, 
            total_rels);

    if (model_opt.labeled) {
        TRACE_LOG("LAS: %.4lf ( %d / %d )", 
                (double)label_correct / total_rels, 
                label_correct, 
                total_rels);
    }

    holdout_dat.clear();
}

}   //  end for namespace parser
}   //  end for namespace ltp