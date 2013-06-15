#ifndef __DECODER_2_O_H__
#define __DECODER_2_O_H__

#include "decoder.h"

namespace ltp {
namespace parser {

class Decoder2O : public Decoder {
public:
    Decoder2O(int _L) : L(_L) {}

protected:
    void init_lattice(const Instance * inst) {
        int len = inst->size();
        _lattice_cmp.resize(len, len, len);
        _lattice_incmp.resize(len, len, L);

        for (int i = 0; i < len; ++ i) {
            _lattice_cmp[i][i][i] = new LatticeItem(i);
        }
    }

    void decode_projective(const Instance * inst) {
        int len = inst->size();
        for (int width = 1; width < len; ++ width) {
            for (int s = 0; s + width < len; ++ s) {
                int t = s + witdh;
                for (int l = 0; l < L; ++ l) {
                    _lattice_incmp[s][t][l] = NULL;
                    _lattice_incmp[t][s][l] = NULL;
                }

                for (int m = s; m <= t; ++ m) {
                    _lattice_cmp[s][t][m] = NULL;
                    _lattice_cmp[t][s][m] = NULL;
                }

                for (int l = 0; l < L; ++ l) {
                    for (int r = s; r < t; ++ r) {
                        if (r == s) {
                            continue;
                        }

                        if (r + 1 == t) {
                            continue;
                        }

                        const LatticeItem * best_left_item_st = NULL;
                    }
                }
            }
        }
    }

private:
    int L;
    Mat3< const LatticeItem * > _lattice_cmp;
    Mat3< const LatticeItem * > _lattice_incmp;
};

}   //  end for namespace parser
}   //  end for namespace ltp

#endif  //  end for __DECODER_2_O__
