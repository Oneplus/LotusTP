#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <iostream>

namespace ltp {
namespace parser {

using namespace std;

struct TrainOptions {
    string  train_file;             /*< the training file */
    string  holdout_file;           /*< the develop file */
    int     max_iter;               /*< the iteration number */
    // training algorithm parameter
    string  algorithm;              /*< the algorithm */
    string  model_name;             /*< the model name */
    bool    conservative_update;    /*< conservative update */
};

struct TestOptions {
    string test_file;
    string model_file;
};

struct FeatureOptions {
    bool use_postag;
    bool use_postag_unigram;
    bool use_postag_bigram;
    bool use_postag_chars;      // template: pos+chars

    // dependency feature group
    bool use_dependency;
    bool use_dependency_unigram;
    bool use_dependency_bigram;
    bool use_dependency_surrounding;
    bool use_dependency_between;

    // automaticall calculate
    bool use_distance_in_dependency_features;
    bool use_unlabeled_dependency;
    bool use_labeled_dependency;

    bool use_lemma;
    bool use_coarse_postag;
    bool use_sibling;
    bool use_grand;
};

struct ModelOptions {
    bool    labeled;
    string  decoder_name;           /*< the training order */
    int     display_interval;       /*< the display interval */
};

ModelOptions     model_opt;
TrainOptions     train_opt;
TestOptions      test_opt;
FeatureOptions   feat_opt;

}       //  end for namespace parser
}       //  end for namespace ltp


#endif  //  end for __OPTIONS_H__
