#include "decoder2o.h"
#include "options.h"

namespace ltp {
namespace parser {

void Decoder2O::init_lattice(const Instance * inst) {
    int len = inst->size();
    _lattice_cmp.resize(len, len);
    _lattice_sib.resize(len, len);
    _lattice_incmp.resize(len, len);

    for (int i = 0; i < len; ++ i) {
        for (int j = 0; j < len; ++ j) {
            _lattice_cmp[i][j] = 0;
            _lattice_sib[i][j] = 0;
            _lattice_incmp[i][j] = 0;
        }
    }
    for (int i = 0; i < len; ++ i) {
        _lattice_cmp[i][i] = new LatticeItem(i);
    }
}

void Decoder2O::decode_projective(const Instance * inst) {
    int len = inst->size();
    for (int width = 1; width < len; ++ width) {
        for (int s = 0; s + width < len; ++ s) {
            int t = s + width;

            for (int l = 0; l < L; ++ l) {
                double shared_score = 0.;

                if (feat_opt.use_unlabeled_dependency) {
                    shared_score += inst->dependency_scores[s][t];
                }

                if (feat_opt.use_labeled_dependency) {
                    shared_score += inst->labeled_dependency_scores[s][t][l];
                }

                {   // I(s,t) = C(s,s) + C(t,s+1)
                    const LatticeItem * const left  = _lattice_cmp[s][s];
                    const LatticeItem * const right = _lattice_cmp[t][s + 1];

                    if (!left || !right) {
                        continue;
                    }

                    double score = left->_prob + right->_prob + shared_score;
                    if (feat_opt.use_unlabeled_sibling) {
                        score += inst->sibling_scores[s][t][s];
                    }

                    if (feat_opt.use_labeled_sibling) {
                        score += inst->labeled_sibling_scores[s][t][s][l];
                    }

                    const LatticeItem * const item = new LatticeItem(INCMP,
                            s,
                            t,
                            score,
                            left,
                            right,
                            l);

                    lattice_insert(_lattice_incmp[s][t], item);
                }   //  end for I(s,t) = C(s,s) + C(t,s+1)

                {   // I(s,t) = I(s,r) + S(r,t)
                    for (int r = s + 1; r < t; ++ r) {
                        const LatticeItem * const left = _lattice_incmp[s][r];
                        const LatticeItem * const right = _lattice_sib[r][t];

                        if (!left || !right) {
                            continue;
                        }

                        double score = left->_prob + right->_prob + shared_score;

                        if (feat_opt.use_unlabeled_sibling) {
                            score += inst->sibling_scores[s][t][r];
                        }

                        if (feat_opt.use_labeled_sibling) {
                            score += inst->labeled_sibling_scores[s][t][r][l];
                        }

                        const LatticeItem * const item = new LatticeItem(INCMP,
                                s,
                                t,
                                score,
                                left,
                                right,
                                l);

                        lattice_insert(_lattice_incmp[s][t], item);
                    }
                }   //  end for I(s,t) = I(s,r) + S(r,t)

            }   // end for for (l = 0; l < L; ++ l)
 
            if (s != 0) {   //  I(t,s) = C(s, t-1) + C(t, t)
                for (int l = 0; l < L; ++ l) {
                    double shared_score = 0.;
 
                    if (feat_opt.use_unlabeled_dependency) {
                        shared_score += inst->dependency_scores[t][s];
                    }

                    if (feat_opt.use_labeled_dependency) {
                        shared_score += inst->labeled_dependency_scores[t][s][l];
                    }

                    {   //  I(t,s) = C(s,t-1) + C(t,t)
                        const LatticeItem * const left = _lattice_cmp[s][t-1];
                        const LatticeItem * const right = _lattice_cmp[t][t];

                        if (!left || !right) {
                            continue;
                        }

                        double score =  left->_prob + right->_prob + shared_score;

                        if (feat_opt.use_unlabeled_sibling) {
                            score += inst->sibling_scores[t][s][t];
                        }

                        if (feat_opt.use_labeled_sibling) {
                            score += inst->labeled_sibling_scores[t][s][t][l];
                        }

                        const LatticeItem * const item = new LatticeItem(INCMP,
                                t,
                                s,
                                score,
                                left,
                                right,
                                l);

                        lattice_insert(_lattice_incmp[t][s], item);
                    }   //  end for I(t, s) = C(s,t-1) + C(t,t)

                    {   //  I(t,s) = S(s,r) + I(t,r)
                        for (int r = s + 1; r < t; ++ r) {
                            const LatticeItem * const left = _lattice_sib[s][r];
                            const LatticeItem * const right = _lattice_incmp[t][r];

                            if (!left || !right) {
                                continue;
                            }

                            double score = left->_prob + right->_prob + shared_score;

                            if (feat_opt.use_unlabeled_sibling) {
                                score += inst->sibling_scores[t][s][r];
                            }

                            if (feat_opt.use_labeled_sibling) {
                                score += inst->labeled_sibling_scores[t][s][r][l];
                            }

                            const LatticeItem * const item = new LatticeItem(INCMP,
                                    t,
                                    s,
                                    score,
                                    left,
                                    right,
                                    l);

                            lattice_insert(_lattice_incmp[t][s], item);
                        }
                    }   //  end for I(t,s) = S(s,r) + I(t,r)

                }
            }   //  end for if (s != 0)
 
            {   // S(s,t) = C(s,r) + C(t,r+1)
                for (int r = s; r < t; ++ r) {
                    const LatticeItem * const left = _lattice_cmp[s][r];
                    const LatticeItem * const right = _lattice_cmp[t][r+1];

                    if (!left || !right) {
                        continue;
                    }

                    double score = left->_prob + right->_prob;

                    const LatticeItem * const item = new LatticeItem(SIBSP,
                            s,
                            t,
                            score,
                            left,
                            right);

                    lattice_insert(_lattice_sib[s][t], item);
                }
            }   // end for S(s,t) = C(s,t) + C(t,r+1)

            {   //  C(s,t) = I(s,r) + C(r,t)
                for (int r = s + 1; r <= t; ++ r) {
                    const LatticeItem * const left = _lattice_incmp[s][r];
                    const LatticeItem * const right = _lattice_cmp[r][t];

                    if (!left || !right) {
                        continue;
                    }

                    double score = left->_prob + right->_prob;

                    if (feat_opt.use_last_sibling) {
                        if (feat_opt.use_unlabeled_sibling) {
                            score += inst->sibling_scores[s][r][r];
                        }

                        if (feat_opt.use_labeled_sibling) {
                            int l = left->_label_s_t;
                            score += inst->labeled_sibling_scores[s][r][r][l];
                        }
                    }

                    const LatticeItem * const item = new LatticeItem(CMP,
                            s,
                            t,
                            score,
                            left,
                            right);

                    lattice_insert(_lattice_cmp[s][t], item);

                }
            }

            if (s != 0) {
                for (int r = s; r < t; ++ r) {
                    const LatticeItem * const left = _lattice_cmp[r][s];
                    const LatticeItem * const right = _lattice_incmp[t][r];

                    if (!left || !right) {
                        continue;
                    }

                    double score = left->_prob + right->_prob;

                    if (feat_opt.use_last_sibling) {
                        if (feat_opt.use_unlabeled_sibling) {
                            score += inst->sibling_scores[t][r][r];
                        }

                        if (feat_opt.use_labeled_sibling) {
                            int l = right->_label_s_t;
                            score += inst->labeled_sibling_scores[t][r][r][l];
                        }
                    }

                    const LatticeItem * const item = new LatticeItem(CMP,
                            t,
                            s,
                            score,
                            left,
                            right);

                    lattice_insert(_lattice_cmp[t][s], item);
                }
            }
        }
    }
}

void Decoder2O::get_result(Instance * inst) {
    int len = inst->size();
    inst->predicted_heads.resize(len, -1);
    if (model_opt.labeled) {
        inst->predicted_deprelsidx.resize(len, -1);
    }

    const LatticeItem * best_item = _lattice_cmp[0][len - 1];
    __BUILD_TREE(inst, best_item);
}

void Decoder2O::free_lattice() {
    int len = _lattice_cmp.nrows();
    for (int i = 0; i < len; ++ i) {
        for (int j = 0; j < len; ++ j) {
            if (_lattice_incmp[i][j]) {
                delete _lattice_incmp[i][j];
            }

            if (_lattice_cmp[i][j]) {
                delete _lattice_cmp[i][j];
            }

            if (_lattice_sib[i][j]) {
                delete _lattice_sib[i][j];
            }
        }
    }
}


void Decoder2O::__BUILD_TREE(Instance * inst, const LatticeItem * item) {
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
    } else if (SIBSP == item->_comp) {
        //  do nothing
    } else {
    }

    __BUILD_TREE(inst, item->_right);
}

}   //  end for namespace parser
}   //  end for namespace ltp
