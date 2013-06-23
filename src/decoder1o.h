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
    void init_lattice(const Instance * inst); 
    void decode_projective(const Instance * inst);
    void get_result(Instance * inst);
    void free_lattice();
private:
    void __BUILD_TREE(Instance * inst, const LatticeItem * item) {
        if (!item) {
            return;
        }

        __BUILD_TREE(inst, item->_left);

        if (INCMP == item->_comp) {
            inst->predicted_heads[item->_t] = item->_s;

            if (model_opt.labeled) {
                inst->predicted_deprelsidx[item->_t] = item->_label_s_t;
            }
        } else if (CMP == item->_comp) {
            // do nothing;
        } else {
        }


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
