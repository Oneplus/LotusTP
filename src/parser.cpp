#include "parser.h"
#include "options.h"

namespace ltp {
namespace parser {

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

}   //  end for namespace parser
}   //  end for namespace ltp
