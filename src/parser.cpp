#include "parser.h"
#include "options.h"
#include "decoder1o.h"
#include "decoder2o.h"
#include "conllreader.h"
#include "conllwriter.h"

#include "treeutils.hpp"

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

    feat_opt.use_sibling                =   false;
    feat_opt.use_sibling_basic          =   false;
    feat_opt.use_sibling_linear         =   false;

    feat_opt.use_last_sibling           =   false;
    feat_opt.use_distance_in_features   =   true;
    // feat_opt.use_distance_in_features = false;

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

        if (cfg.get_integer("feature", "use-sibling", intbuf)) {
            feat_opt.use_sibling = (intbuf == 1);
        }

        if (cfg.get_integer("feature", "use-sibling-basic", intbuf)) {
            feat_opt.use_sibling_basic = (intbuf == 1);
        }

        if (cfg.get_integer("feature", "use-sibling-linear", intbuf)) {
            feat_opt.use_sibling_linear = (intbuf == 1);
        }

        // detrieve sibling type from configuration
        feat_opt.use_unlabeled_sibling = (model_opt.labeled == false &&
                feat_opt.use_sibling);

        feat_opt.use_labeled_sibling = (model_opt.labeled == true &&
                feat_opt.use_sibling);
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

void Parser::collect_unlabeled_features_of_one_instance(Instance * inst,
        const vector<int> & heads,
        SparseVec & vec ) {

    vec.zero();
    if (feat_opt.use_dependency) {
        for (treeutils::DEPIterator itx(heads); !itx.end(); ++ itx) {
            int hid = itx.hid();
            int cid = itx.cid();

            vec.add(inst->dependency_features[hid][cid], 1.);
        }
    }

    if (feat_opt.use_sibling) {
        for (treeutils::SIBIterator itx(heads, feat_opt.use_last_sibling); !itx.end(); ++ itx) {
            int hid = itx.hid();
            int cid = itx.cid();
            int sid = itx.sid();

            vec.add(inst->sibling_features[hid][cid][sid], 1.);
        }
    }
}

void Parser::collect_labeled_features_of_one_instance(Instance * inst,
        const vector<int> & heads,
        const vector<int> & deprelsidx,
        SparseVec & vec) {

    vec.zero();
    if (feat_opt.use_dependency) {
        for (treeutils::DEPIterator itx(heads); !itx.end(); ++ itx) {
            int hid = itx.hid();
            int cid = itx.cid();
            int relidx = deprelsidx[cid];

            vec.add(inst->labeled_dependency_features[hid][cid][relidx], 1.);
        }
    }

    if (feat_opt.use_sibling) {
        for (treeutils::SIBIterator itx(heads, feat_opt.use_last_sibling); !itx.end(); ++ itx) {
            int hid = itx.hid();
            int cid = itx.cid();
            int sid = itx.sid();
            int relidx = deprelsidx[cid];

            vec.add(inst->labeled_sibling_features[hid][cid][sid][relidx], 1.);
        }
    }
}

void Parser::collect_features_of_one_instance(Instance * inst, bool gold) {
    if (gold) {
        if (!model_opt.labeled) {
            collect_unlabeled_features_of_one_instance(inst,
                    inst->heads,
                    inst->features);
        } else {
            collect_labeled_features_of_one_instance(inst,
                    inst->heads,
                    inst->deprelsidx,
                    inst->features);
        }
    } else {
        if (!model_opt.labeled) {
            collect_unlabeled_features_of_one_instance(inst,
                    inst->predicted_heads,
                    inst->predicted_features);
        } else {
            collect_labeled_features_of_one_instance(inst,
                    inst->predicted_heads,
                    inst->predicted_deprelsidx,
                    inst->predicted_features);
        }
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
    int L   = model->num_deprels();
    FeatureSpace& space = model->space;

    if (feat_opt.use_dependency) {

        if (!model_opt.labeled) {
            inst->dependency_features.resize(len, len);
            inst->dependency_scores.resize(len, len);

            inst->dependency_features = 0;
            inst->dependency_scores = DOUBLE_NEG_INF;
        } else {
            inst->labeled_dependency_features.resize(len, len, L);
            inst->labeled_dependency_scores.resize(len, len, L);

            inst->labeled_dependency_features = 0;
            inst->labeled_dependency_scores = DOUBLE_NEG_INF;
        }

        vector< StringVec >  cache;
        vector< int >     cache_again;

        int N = DEPExtractor::num_templates();

        cache.resize( N );

        for (treeutils::DEPTreeSpaceIterator itx(len); !itx.end(); ++ itx) {
            int hid = itx.hid();
            int cid = itx.cid();

            // here the self-implementated String Vector is little
            // fasteer than the list<string>
            for (int i = 0; i < N; ++ i) {
                cache[i].clear();
            }

            DEPExtractor::extract2o(inst, hid, cid, cache);
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

            if (num_feat > 0) {
                if (!model_opt.labeled) {
                    inst->dependency_features[hid][cid] = new FeatureVector;
                    inst->dependency_features[hid][cid]->n = num_feat;
                    inst->dependency_features[hid][cid]->idx = 0;
                    inst->dependency_features[hid][cid]->val = 0;

                    inst->dependency_features[hid][cid]->idx = new int[num_feat];
                    for (int j = 0; j < num_feat; ++ j) {
                        inst->dependency_features[hid][cid]->idx[j] = cache_again[j];
                    }
                } else {
                    for (int l = 0; l < L; ++ l) {
                        inst->labeled_dependency_features[hid][cid][l] = new FeatureVector;
                        inst->labeled_dependency_features[hid][cid][l]->n = num_feat;
                        inst->labeled_dependency_features[hid][cid][l]->idx = 0;
                        inst->labeled_dependency_features[hid][cid][l]->val = 0;

                        inst->labeled_dependency_features[hid][cid][l]->idx = new int[num_feat];
                        for (int j = 0; j < num_feat; ++ j) {
                            inst->labeled_dependency_features[hid][cid][l]->idx[j] = (
                                    cache_again[j] + l);
                        }
                    }
                }
            }
        }   //  end for DEPTreeSpaceIterator itx
    }   //  end for feat_opt.use_dependency

    if (feat_opt.use_sibling) {
        if (!model_opt.labeled) {
            inst->sibling_features.resize(len, len, len);
            inst->sibling_scores.resize(len, len, len);

            inst->sibling_features = 0;
            inst->sibling_scores = DOUBLE_NEG_INF;
        } else {
            inst->labeled_sibling_features.resize(len, len, len, L);
            inst->labeled_sibling_scores.resize(len, len, len, L);

            inst->labeled_sibling_features = 0;
            inst->labeled_sibling_scores = DOUBLE_NEG_INF;
        }

        int N = SIBExtractor::num_templates();

        vector< StringVec > cache;
        vector< int > cache_again;

        cache.resize(N);

        for (treeutils::SIBTreeSpaceIterator itx(len, feat_opt.use_last_sibling); !itx.end(); ++ itx) {
            int hid = itx.hid();
            int cid = itx.cid();
            int sid = itx.sid();

            for (int i = 0; i < N; ++ i) {
                cache[i].clear();
            }

            SIBExtractor::extract3o(inst, hid, cid, sid, cache);
            cache_again.clear();

            //
            for (int tid = 0; tid < cache.size(); ++ tid) {
                for (int itx = 0; itx < cache[tid].size(); ++ itx) {
                    int idx = model->space.index(FeatureSpace::SIB, tid, cache[tid][itx]);
                    if (idx >= 0) {
                        cache_again.push_back(idx);
                    }
                }
            }

            int num_feat = cache_again.size();

            if (num_feat > 0) {
                if (!model_opt.labeled) {
                    inst->sibling_features[hid][cid][sid] = new FeatureVector;
                    inst->sibling_features[hid][cid][sid]->n = num_feat;
                    inst->sibling_features[hid][cid][sid]->idx = 0;
                    inst->sibling_features[hid][cid][sid]->val = 0;

                    inst->sibling_features[hid][cid][sid]->idx = new int[num_feat];
                    for (int j = 0; j < num_feat; ++ j) {
                        inst->sibling_features[hid][cid][sid]->idx[j] = cache_again[j];
                    }
                } else {
                    for (int l = 0; l < L; ++ l) {
                        inst->labeled_sibling_features[hid][cid][sid][l] = new FeatureVector;
                        inst->labeled_sibling_features[hid][cid][sid][l]->n = num_feat;
                        inst->labeled_sibling_features[hid][cid][sid][l]->idx = 0;
                        inst->labeled_sibling_features[hid][cid][sid][l]->val = 0;

                        inst->labeled_sibling_features[hid][cid][sid][l]->idx = new int[num_feat];
                        for (int j = 0; j < num_feat; ++ j) {
                            inst->labeled_sibling_features[hid][cid][sid][l]->idx[j] = (
                                    cache_again[j] + l);
                        }
                    }   //  end for if model_opt.labeled
                }
            }
        }   //  end for SIBTreeSpaceIterator itx
    }   //  end for feat_opt.use_sibling
}

void Parser::extract_features(vector<Instance *>& dat) {
    ofstream fout("lgdpj.fv.tmp", std::ofstream::binary);
    // DependencyExtractor
    for (int i = 0; i < dat.size(); ++ i) {
        extract_features(dat[i]);

        dat[i]->dump_all_featurevec(fout);
        if ((i + 1) % model_opt.display_interval == 0) {
            TRACE_LOG("[%d] instance is extracted.", i + 1);
        }
    }   // end for i = 0; i < dat.size(); ++ i

    fout.close();
}

void Parser::build_gold_features() {
    ifstream fin("lgdpj.fv.tmp", std::ifstream::binary);
    for (int i = 0; i < train_dat.size(); ++ i) {
        train_dat[i]->load_all_featurevec(fin);
        collect_features_of_one_instance(train_dat[i], true);
        train_dat[i]->nice_all_featurevec();
    }
    fin.close();
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

    model->param.realloc(model->dim());
    TRACE_LOG("Allocate a parameter vector of [%d] dimension.", model->dim());

    if (model_opt.decoder_name == "1o") {
        if (!model_opt.labeled) {
            decoder = new Decoder1O();
            TRACE_LOG("1st-Order Decoder without label is configured.");
        } else {
            decoder = new Decoder1O(model->num_deprels());
            TRACE_LOG("1st-Order Decoder with [%d] labels is configured.", model->num_deprels());
        }
    } else if (model_opt.decoder_name == "2o-sib") {
        if (!model_opt.labeled) {
            decoder = new Decoder2O();
            TRACE_LOG("2nd-Order Decoder without label is configured.");
        } else {
            decoder = new Decoder2O(model->num_deprels());
            TRACE_LOG("2nd-Order Decoder with [%d] labels is configured.", model->num_deprels());
        }
    }

    // ifstream fin("lgdpj.fv.tmp", std::ifstream::binary);
    for (int iter = 0; iter < train_opt.max_iter; ++ iter) {
        TRACE_LOG("Start training epoch #%d.", (iter + 1));
        // fin.seekg(0, fin.beg);

        // random_shuffle(train_dat.begin(), train_dat.end());
        for (int i = 0; i < train_dat.size(); ++ i) {
            // train_dat[i]->load_all_featurevec(fin);

            extract_features(train_dat[i]);
            calculate_score(train_dat[i], model->param);
            decoder->decode(train_dat[i]);
            collect_features_of_one_instance(train_dat[i], true);
            collect_features_of_one_instance(train_dat[i], false);
            // train_dat[i]->nice_all_featurevec();

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

            train_dat[i]->cleanup();
        }

        model->param.flush( train_dat.size() * (iter + 1) );
        evaluate();

        ofstream fout((train_opt.model_name + "." + to_str(iter) + ".model").c_str(),
                std::ofstream::binary);
        model->save(fout);

    }

    delete model;
}

void Parser::evaluate(void) {
    const char * holdout_file = train_opt.holdout_file.c_str();

    int head_correct = 0;
    int label_correct = 0;
    int total_rels = 0;

    ifstream f(holdout_file);
    CoNLLReader reader(f);

    Instance * inst = NULL;

    double before = get_time();
    while ((inst = reader.next())) {

        if (model_opt.labeled) {
            inst->deprelsidx.resize(inst->size());
            for (int i = 1; i < inst->size(); ++ i) {
                inst->deprelsidx[i] = model->deprels.index(inst->deprels[i].c_str());
            }
        }

        extract_features(inst);
        calculate_score(inst, model->param, true);

        decoder->decode(inst);

        total_rels += inst->num_rels();
        head_correct += inst->num_correct_heads();
        label_correct += inst->num_correct_heads_and_labels();

        delete inst;
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

    double after = get_time();
    TRACE_LOG("consuming time: %.2lf", after - before);

    // holdout_dat.clear();
}

void Parser::test() {
    double before = get_time();
    const char * model_file = test_opt.model_file.c_str();
    ifstream mfs(model_file, std::ifstream::binary);

    if (!mfs) {
        ERROR_LOG("Failed to open file [%s].", model_file);
        return;
    }

    model = new Model;
    if (!model->load(mfs)) {
        ERROR_LOG("Failed to load model");
        return;
    }

    TRACE_LOG("Number of postags [%d]", model->num_postags());
    TRACE_LOG("Number of deprels [%d]", model->num_deprels());
    TRACE_LOG("Number of features [%d]", model->num_features());
    TRACE_LOG("Number of dimension [%d]", model->dim());
    TRACE_LOG("Labeled:                         %s", 
            (model_opt.labeled ? "true" : "fales"));
    TRACE_LOG("Decoder:                         %s", 
            model_opt.decoder_name.c_str());
    TRACE_LOG("Dependency features:             %s",
            (feat_opt.use_dependency ? "true" : "false"));
    TRACE_LOG("Dependency features unigram:     %s", 
            (feat_opt.use_dependency_unigram ? "true" : "false"));
    TRACE_LOG("Dependency features bigram:      %s", 
            (feat_opt.use_dependency_bigram ? "true" : "false"));
    TRACE_LOG("Dependency features surrounding: %s", 
            (feat_opt.use_dependency_surrounding ? "true" : "false"));
    TRACE_LOG("Dependency features between:     %s", 
            (feat_opt.use_dependency_between ? "true" : "false"));
    TRACE_LOG("Sibling features:                %s",
            (feat_opt.use_sibling ? "true" : "false"));
    TRACE_LOG("Sibling basic features:          %s", 
            (feat_opt.use_sibling_basic ? "true" : "false"));
    TRACE_LOG("Sibling linear features:         %s", 
            (feat_opt.use_sibling_linear ? "true" : "false"));

    const char * test_file = test_opt.test_file.c_str();

    ifstream f(test_file);
    if (!f) {
        return;
    }

    CoNLLReader reader(f);

    Instance * inst = NULL;

    if (model_opt.decoder_name == "1o") {
        if (!model_opt.labeled) {
            decoder = new Decoder1O();
        } else {
            decoder = new Decoder1O(model->num_deprels());
        }
    } else if (model_opt.decoder_name == "2o-sib") {
        if (!model_opt.labeled) {
            decoder = new Decoder2O();
        } else {
            decoder = new Decoder2O(model->num_deprels());
        }
    }

    cerr << get_time() - before << endl;
    before = get_time();
    while ((inst = reader.next())) {
        extract_features(inst);
        /*int len = inst->labeled_dependency_features.total_size();
        for (int i = 0; i < len; ++ i) {
            cout << inst->labeled_dependency_features.c_buf()[i] << endl;
        }*/
        calculate_score(inst, model->param);

        decoder->decode(inst);

        instance_verify(inst, cout, true);

        delete inst;
    }
    double after = get_time();
    cerr << after - before << endl;
    sleep(1000000);
}


void Parser::calculate_score(Instance * inst, const Parameters& param, bool use_avg) {
    int len = inst->size();
    int L = model->num_deprels();

    if (feat_opt.use_postag_unigram) {
        for (int i = 0; i < len; ++ i) {
            inst->postag_unigram_scores[i] = 0.;
            FeatureVector * fv = inst->postag_unigram_features[i];
            inst->postag_unigram_scores[i] = param.dot(fv, false);
        }
    }   //  end if feat_opt.use_postag_unigram

    if (feat_opt.use_unlabeled_dependency) {
        for (treeutils::DEPTreeSpaceIterator itx(len); !itx.end(); ++ itx) {
            int hid = itx.hid();
            int cid = itx.cid();

            FeatureVector * fv = inst->dependency_features[hid][cid];
            inst->dependency_scores[hid][cid] = 0.;

            if (!fv) {
                continue;
            }

            inst->dependency_scores[hid][cid] = param.dot(fv, use_avg);
        }
    }   //  end if feat_opt.use_unlabeled_dependency

    if (feat_opt.use_labeled_dependency) {
         for (treeutils::DEPTreeSpaceIterator itx(len); !itx.end(); ++ itx) {
             int hid = itx.hid();
             int cid = itx.cid();
             for (int l = 0; l < L; ++ l) {
                 FeatureVector * fv = inst->labeled_dependency_features[hid][cid][l];
                 inst->labeled_dependency_scores[hid][cid][l] = 0.;

                 if (!fv) {
                     continue;
                 }

                 inst->labeled_dependency_scores[hid][cid][l] = param.dot(fv, use_avg);
            }
        }
    }   //  end if feat_opt.use_labeled_dependency

    if (feat_opt.use_unlabeled_sibling) {
        for (treeutils::SIBTreeSpaceIterator itx(len, feat_opt.use_last_sibling); !itx.end(); ++ itx) {
            int hid = itx.hid();
            int cid = itx.cid();
            int sid = itx.sid();

            FeatureVector * fv = inst->sibling_features[hid][cid][sid];
            inst->sibling_scores[hid][cid][sid] = 0.;

            if (!fv) {
                continue;
            }

            inst->sibling_scores[hid][cid][sid] = param.dot(fv, use_avg);
        }
    }   //  end for if feat_opt.use_unlabeled_sibling

    if (feat_opt.use_labeled_sibling) {
        for (treeutils::SIBTreeSpaceIterator itx(len, feat_opt.use_last_sibling); !itx.end(); ++ itx) {
            int hid = itx.hid();
            int cid = itx.cid();
            int sid = itx.sid();

            for (int l = 0; l < L; ++ l) {
                FeatureVector * fv = inst->labeled_sibling_features[hid][cid][sid][l];
                inst->labeled_sibling_scores[hid][cid][sid][l] = 0.;

                if (!fv) {
                    continue;
                }

                inst->labeled_sibling_scores[hid][cid][sid][l] = param.dot(fv, use_avg);
            }
        }
    }   //  end for if feat_opt.use_labeled_sibling
}

}   //  end for namespace parser
}   //  end for namespace ltp
