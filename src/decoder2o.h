#ifndef __DECODER_2_O_H__
#define __DECODER_2_O_H__

#include "decoder.h"

namespace ltp {
namespace parser {

class Decoder2O : public Decoder {
public:
    Decoder2O(int _L = 1) : L(_L) {}

public:
    void init_lattice(const Instance * inst);
    void decode_projective(const Instance * inst);
    void get_result(Instance * inst);
    void free_lattice();
private:
    int L;
    Mat< const LatticeItem * > _lattice_cmp;
    Mat< const LatticeItem * > _lattice_incmp;
    Mat< const LatticeItem * > _lattice_sib;

private:
    void __BUILD_TREE(Instance * inst, const LatticeItem * item);
};

}   //  end for namespace parser
}   //  end for namespace ltp

#endif  //  end for __DECODER_2_O__
