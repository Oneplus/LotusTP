#include "decoder1o.h"

namespace ltp {
namespace parser {

void Decoder1O::init_lattice(const Instance * inst) {
    int len = inst->size();
    _lattice_cmp.resize(len, len);
    _lattice_incmp.resize(len, len, L);

    _lattice_cmp = NULL;
    _lattice_incmp = NULL;

    for (int i = 0; i < len; ++ i) {
        _lattice_cmp[i][i] = new LatticeItem(i);
    }
}


void Decoder1O::decode_projective(const Instance * inst) {
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
                        double prob = (left->_prob + right->_prob);

                        if (feat_opt.use_unlabeled_dependency) {
                            prob += inst->dependency_scores[s][t];
                        }

                        if (feat_opt.use_labeled_dependency) {
                            prob += inst->labeled_dependency_scores[s][t][l];
                        }

                        // @NOTE
                        // These two features are used for postag-dependency joint model.
                        // In pipeline model, this part should be left alone.
                        if (feat_opt.use_postag_unigram) {
                        }

                        if (feat_opt.use_postag_bigram) {
                        }

                        // std::cerr << l <<" ";
                        const LatticeItem * const item = new LatticeItem(INCMP,
                                s,
                                t,
                                prob,
                                left,
                                right,
                                l);

                        // cerr << "INCMP " << s << "-" << t << "-" << l << endl;
                        lattice_insert(_lattice_incmp[s][t][l], item);
                    }

                    if (s != 0) {   // I(t,s)
                        double prob = (left->_prob + right->_prob);

                        if (feat_opt.use_unlabeled_dependency) {
                            prob += inst->dependency_scores[t][s];
                        }

                        if (feat_opt.use_labeled_dependency) {
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
                        const LatticeItem * const item = new LatticeItem(CMP,
                                s,
                                t,
                                prob,
                                left,
                                right);

                        // cerr << "CMP " << s << "-" << t << endl;
                        lattice_insert(_lattice_cmp[s][t], item);
                    }
                }   //  end for if (r != s)

                if (r != t && s != 0) { // C(t,s) = I(t,r) + C(r,s)
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
                        const LatticeItem * const item = new LatticeItem(CMP,
                                t,
                                s,
                                prob,
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

void Decoder1O::get_result(Instance * inst) {
    int len = inst->size();
    inst->predicted_heads.resize(len, -1);
    if (model_opt.labeled) {
        inst->predicted_deprelsidx.resize(len, -1);
    }

    const LatticeItem * best_item = _lattice_cmp[0][len - 1];
    __BUILD_TREE(inst, best_item);
}

void Decoder1O::free_lattice() {
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

void Decoder1O::__BUILD_TREE(Instance * inst, const LatticeItem * item) {
    if (!item) {
        return;
    }

    __BUILD_TREE(inst, item->_left);

    if (INCMP == item->_comp) {
        inst->predicted_heads[item->_t] = item->_s;

        if (model_opt.labeled) {
            // std::cout << "label: " << item->_label_s_t << std::endl;
            inst->predicted_deprelsidx[item->_t] = item->_label_s_t;
        }
    } else if (CMP == item->_comp) {
        // do nothing;
    } else {
    }


    __BUILD_TREE(inst, item->_right);
}



}
}

