#include "options.h"
#include "featurespace.h"
#include "extractor.h"

#include "strvec.hpp"
#include "logging.hpp"

namespace ltp {
namespace parser {

using namespace ltp::utility;

int FeatureSpace::retrieve(int gid, int tid, const char * key, bool create) {
    // no boundary check, which is very dangerous
    return groups[gid]->retrieve(tid, key, create);
}

int FeatureSpace::index(int gid, int tid, const char * key, int lid) {
    int bid = groups[gid]->retrieve(tid, key, false);
    if (bid < 0) return -1;

    return bid * _num_deprels + lid + offsets[gid];
}

int FeatureSpace::build_feature_space(int num_deprels, const std::vector<Instance *> & instances) {
    _num_deprels = num_deprels;
    // allocate dictionary groups according to the options
    allocate_dictionary_groups();

    // loop over the training instances and extract gold features.
    for (int i = 0; i < instances.size(); ++ i) {
        Instance * inst = instances[i];

        int len = inst->size();

        for (int j = 1; j < len; ++ j) {
            int hid = inst->heads[j];
            int cid = j;

            if (feat_opt.use_dependency) {
                //std::vector< std::list<std::string> > cache;
                int N = DEPExtractor::num_templates();
                std::vector< StringVec > cache;
                cache.resize( N );

                DEPExtractor::extractor()->extract2o(inst, hid, cid, cache);
                for (int k = 0; k < cache.size(); ++ k) {
                    /*for (std::list<std::string>::iterator itx = cache[k].begin();
                            itx != cache[k].end();
                            ++ itx) {*/
                    for (int itx = 0; itx < cache[k].size(); ++ itx) {
                        retrieve(DEP, k, cache[k][itx], true);
                    }
                }
            }
        }

        if ((i+1) % model_opt.display_interval== 0) {
            TRACE_LOG("In building feature space, [%d] instances scanned.", i+1);
        }
    }

    _offset = 0;
    _num_features = 0;

    offsets[DEP] = _offset;
    if (feat_opt.use_dependency) {
        _num_features += groups[DEP]->dim();
        _offset += groups[DEP]->dim() * _num_deprels;
    }

    /*offsets[SIB] = offset;
    if (feat_opt.use_sibling) {
        offset += groups[SIB]->dim() * _num_deprels;
    }

    offsets[GRD] = offset;
    if (feat_opt.use_grand) {
        offset += groups[GRD]->dim() * _num_deprels;
    }

    offsets[GRDSIB] = offset;
    if (feat_opt.use_grandsibling) {
        offset += groups[GRDSIB]->dim() * _num_deprels;
    }*/
}

int FeatureSpace::allocate_dictionary_groups() {
    int ret = 0;
    if (feat_opt.use_dependency) {
        groups[DEP] = new DictionaryCollections( DEPExtractor::num_templates() );
        ++ ret;
    }

    /*if (feat_opt.use_sibling) {
        groups[SIB] = new DictionaryGroup( SIBExtractor::num_templates() );
        ++ ret;
    }

    if (feat_opt.use_grand) {
        groups[GRD] = new DictionaryGroup( GRDExtractor::num_templates() );
        ++ ret;
    }

    if (feat_opt.use_grand_sibling) {
        groups[GRDSIB] = new DictionaryGroup( GRDSIBExtractor::num_template() );
        ++ ret;
    }

    if (feat_opt.use_postag_unigram) {
        groups[POSU] = new DictionaryGroup( POSUExtractor::num_template() );
        ++ ret;
    }

    if (feat_opt.use_postag_bigram) {
        groups[POSB] = new DictionaryGroup( POSBExtractor::num_template() );
        ++ ret;
    }*/

    return ret;
}

int FeatureSpace::num_features() {
    return _num_features;
}

int FeatureSpace::dim() {
    return _offset;
}

void FeatureSpace::save(std::ostream & out) {
    if (feat_opt.use_dependency) {
        groups[DEP]->dump(out);
    }

    /*if (feat_opt.use_sibling) {
        groups[SIB]->dump(out);
    }

    if (feat_opt.use_grand) {
        groups[GRD]->dump(out);
    }

    if (feat_opt.use_grand_sibling) {
        groups[GRDSIB]->dump(out);
    }

    if (feat_opt.use_postag_unigram) {
        groups[POSU]->dump(out);
    }

    if (feat_opt.use_postag_bigram) {
        groups[POSB]->dump(out);
    }*/
}

int FeatureSpace::load(std::istream & in) {
    int ret = 0;

    if (feat_opt.use_dependency) {
        groups[DEP] = new DictionaryCollections( DEPExtractor::num_templates() );
        if (!groups[DEP]->load(in)) {
            return -1;
        }
        ++ ret;
    }

    /*if (feat_opt.use_sibling) {
        groups[SIB] = new DictionaryGroup( SIBExtractor::num_templates() );
        ++ ret;
    }

    if (feat_opt.use_grand) {
        groups[GRD] = new DictionaryGroup( GRDExtractor::num_templates() );
        ++ ret;
    }

    if (feat_opt.use_grand_sibling) {
        groups[GRDSIB] = new DictionaryGroup( GRDSIBExtractor::num_template() );
        ++ ret;
    }

    if (feat_opt.use_postag_unigram) {
        groups[POSU] = new DictionaryGroup( POSUExtractor::num_template() );
        ++ ret;
    }

    if (feat_opt.use_postag_bigram) {
        groups[POSB] = new DictionaryGroup( POSBExtractor::num_template() );
        ++ ret;
    }*/

    return ret;

}

}   //  end for namespace parser
}   //  end for namespace ltp
