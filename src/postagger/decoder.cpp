#include "decoder.h"

namespace ltp {
namespace postagger {


void Decoder::decode(Instance * inst) {
    init_lattice(inst);
    viterbi_decode(inst);
    get_result(inst);
    free_lattice();
}

void Decoder::init_lattice(const Instance * inst) {
    int len = inst->size();
    lattice.resize(len, L);
    lattice = NULL;
}

void Decoder::viterbi_decode(const Instance * inst) {
    int len = inst->size();
    for (int i = 0; i < len; ++ i) {
        for (int l = 0; l < L; ++ l) {
            if (i == 0) {
                LatticeItem * item = new LatticeItem(i, l, inst->uni_scores[i][l], NULL);
                lattice_insert(lattice[i][l], item);
            } else {
                for (int pl = 0; pl < L; ++ pl) {
                    double score = 0.;
                    const LatticeItem * prev = lattice[i-1][pl];

                    if (!prev) {
                        continue;
                    }

                    score = inst->uni_scores[i][l] + inst->bi_scores[pl][l] + prev->score;
                    const LatticeItem * item = new LatticeItem(i, l, score, prev);
                    lattice_insert(lattice[i][l], item);
                }
            }   //  end for if i == 0
        }
    }
}

void Decoder::get_result(Instance * inst) {
    int len = inst->size();
    const LatticeItem * best_item = NULL;
    for (int l = 0; l < L; ++ l) {
        if (best_item == NULL || lattice[len - 1][l]->score > best_item->score) {
            best_item = lattice[len - 1][l];
        }
    }

    const LatticeItem * item = best_item;
    inst->predicted_tagsidx.resize(len);

    while (item) {
        inst->predicted_tagsidx[item->i] = item->l;
        // std::cout << item->i << " " << item->l << std::endl;
        item = item->prev;
    }
}

void Decoder::free_lattice() {
    for (int i = 0; i < lattice.nrows(); ++ i) {
        for (int j = 0; j < lattice.ncols(); ++ j) {
            if (lattice[i][j]) delete lattice[i][j];
        }
    }
}

}       //  end for namespace postagger
}       //  end for namespace ltp

