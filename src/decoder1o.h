#ifndef __DECODER_1_O_H__
#define __DECODER_1_O_H__

#include "instance.h"
#include "decoder.h"
#include "mat.h"
#include "options.h"

#include "debug.h"

namespace ltp {
namespace parser {

using namespace ltp::math;

class Decoder1O : public Decoder {
public:
    Decoder1O(int _L = 1) : L(_L) {}

protected:
    void init_lattice(const Instance * inst) {
        int len = inst->size();
        _lattice_cmp.resize(len, len);
        _lattice_incmp.resize(len, len, L);

        for (int i = 0; i < len; ++ i) {
            _lattice_cmp[i][i] = new LatticeItem(i);
            for (int l = 0; l < L; ++ l) {
                _lattice_incmp[i][i][l] = NULL;
            }
        }
    }

    void decode_projective(const Instance * inst) {
        int len = inst->size();

        // instance_verify(inst);
        for (int width = 1; width < len; ++ width) {
            for (int s = 0; s + width < len; ++ s) {
                int t = s + width;
                _lattice_cmp[s][t] = NULL;
                _lattice_cmp[t][s] = NULL;
                for (int l = 0; l < L; ++ l) {
                    _lattice_incmp[s][t][l] = NULL;
                    _lattice_incmp[t][s][l] = NULL;
                }

                for (int r = s; r < t; ++ r) { 
                    const LatticeItem * const left = _lattice_cmp[s][r];
                    if (!left) {
                        continue;
                    }

                    const LatticeItem * const right = _lattice_cmp[t][r+1];
                    if (!right) {
                        continue;
                    }

                    for (int l = 0; l < L; ++ l) {

                        {   // I(s,t) = C(s,r) + C(t,r+1)
                            list<const FeatureVector *> fvs;
                            double prob = (left->_prob + right->_prob);

                            if (feat_opt.use_unlabeled_dependency) {
                                // fvs.push_back(inst->dependency_features[s][t]);
                                prob += inst->dependency_scores[s][t];
                            }

                            if (feat_opt.use_labeled_dependency) {
                                // fvs.push_back( inst->labeled_dependency_features[s][t][l] );
                                prob += inst->labeled_dependency_scores[s][t][l];
                            }

                            // @NOTE
                            // These two features are used for postag-dependency joint model.
                            // In pipeline model, this part should be left alone.
                            if (feat_opt.use_postag_unigram) {
                            }

                            if (feat_opt.use_postag_bigram) {
                            }

                            const LatticeItem * const item = new LatticeItem(INCMP,
                                    s,
                                    t,
                                    prob,
                                    fvs,
                                    left,
                                    right,
                                    l);

                            // cerr << "INCMP " << s << "-" << t << "-" << l << endl;
                            lattice_insert(_lattice_incmp[s][t][l], item);
                        }

                        if (s != 0) {   // I(t,s)
                            list<const FeatureVector *> fvs;
                            double prob = (left->_prob + right->_prob);

                            if (feat_opt.use_unlabeled_dependency) {
                                // fvs.push_back(inst->dependency_features[t][s]);
                                prob += inst->dependency_scores[t][s];
                            }

                            if (feat_opt.use_labeled_dependency) {
                                // fvs.push_back(inst->labeled_dependency_features[t][s][l]);
                                prob += inst->labeled_dependency_scores[t][s][l];
                            }

                            if (feat_opt.use_postag_unigram) {
                            }

                            if (feat_opt.use_postag_bigram) {
                            }

                            const LatticeItem * const item = new LatticeItem(INCMP,
                                    t,
                                    s,
                                    prob,
                                    fvs,
                                    left,
                                    right,
                                    l);

                            // cerr << "INCMP " << t << "-" << s << "-" << l << endl;
                            lattice_insert(_lattice_incmp[t][s][l], item);
                        }   //  end for if (s != 0)
                    }   //  end for for (int l = 0; l < _L; ++ l)
                }   //  end for for (int r = s; r < t; ++ r)

                for (int r = s; r <= t; ++ r) {
                    if (r != s) {   //  C(s,t) = I(s,r) + C(r,t)
                        const LatticeItem * const right = _lattice_cmp[r][t];
                        if (!right) {
                            continue;
                        }

                        for (int l = 0; l < L; ++ l) {
                            const LatticeItem * const left = _lattice_incmp[s][r][l];
                            if (!left) {
                                continue;
                            }

                            const double prob = left->_prob + right->_prob;
                            list<const FeatureVector *> fvs;
                            const LatticeItem * const item = new LatticeItem(CMP,
                                    s,
                                    t,
                                    prob,
                                    fvs,
                                    left,
                                    right);

                            // cerr << "CMP " << s << "-" << t << endl;
                            lattice_insert(_lattice_cmp[s][t], item);
                        }
                    }   //  end for if (r != s)

                    if (r != t && s != 0) { // C(t,s) = I(t,r) = C(r,s)
                        const LatticeItem * const left = _lattice_cmp[r][s];
                        if (!left) {
                            continue;
                        }

                        for (int l = 0; l < L; ++ l) {
                            const LatticeItem * const right = _lattice_incmp[t][r][l];
                            if (!right) {
                                continue;
                            }

                            const double prob = left->_prob + right->_prob;
                            list<const FeatureVector *> fvs;
                            const LatticeItem * const item = new LatticeItem(CMP,
                                    t,
                                    s,
                                    prob,
                                    fvs,
                                    left,
                                    right);

                            // cerr << "CMP " << t << "-" << s << endl;
                            lattice_insert(_lattice_cmp[t][s], item);
                        }   //  end for for (int l = 0; l < L; ++ l) 
                    }   //  end for if (r != t && s != 0)
                }
            }
        }
    }

    void get_result(Instance * inst) {
        int len = inst->size();
        inst->predicted_heads.resize(len);

        if (model_opt.labeled) {
            inst->predicted_deprels.resize(len);
        }

        const LatticeItem * best_item = _lattice_cmp[0][len - 1];
        __BUILD_TREE(inst, best_item);
    }

    void free_lattice() {
        int len = _lattice_cmp.nrows();
        for (int i = 0; i < len; ++ i) {
            for (int j = 0; j < len; ++ j) {
                for (int l = 0; l < L; ++ l) {
                    if (_lattice_incmp[i][j][l]) {
                        delete _lattice_incmp[i][j][l];
                    }
                }

                delete _lattice_cmp[i][j];
            }
        }
    }

private:
    void __BUILD_TREE(Instance * inst, const LatticeItem * item) {
        if (!item) {
            return;
        }

        __BUILD_TREE(inst, item->_left);

        if (INCMP == item->_comp) {
            inst->predicted_heads[item->_t] = item->_s;
        } else if (CMP == item->_comp) {
            // do nothing;
        } else {
        }

        /*for (list<const FeatureVector *>::const_iterator itx = item->_fvs.begin();
                itx != item->_fvs.end();
                itx ++ ) {
            inst->predicted_features.add(*itx);
        }*/

        __BUILD_TREE(inst, item->_right);
    }

protected:
    int L;

    Mat< const LatticeItem * >    _lattice_cmp;   //  complete span
    Mat3< const LatticeItem * >   _lattice_incmp; //  incomplete span
};      //  end for class Decoder1O

}       //  end for namespace parser
}       //  end for namespace ltp

#endif  //  end for __DECODER_1_O_H__
