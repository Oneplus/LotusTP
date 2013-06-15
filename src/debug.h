#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "instance.h"

namespace ltp {
namespace parser {

void instance_verify(const Instance * inst, ostream & out, bool show_features) {
    out << "instance : {" << endl;
    out << "  basic: {" << endl;
    for (int i = 0; i < inst->size(); ++ i) {
        out << "    [" << i << "]\t" 
            << inst->forms[i] 
            << "\t"
            << inst->postags[i]
            << "\t"
            << inst->heads[i];

        if (inst->predicted_heads.size() > 0) {
            out << "("
                << inst->predicted_heads[i]
                << ")";
        }
        out << endl;
    }
    out << "  }," << endl;
    if (show_features) {
        out << "  gold-features: ";
        inst->features.str(out, "    ");

        out << "  predicted-features: ";
        inst->predicted_features.str(out, "    ");
    }

    out << "}" << endl;
}

}       //  end for namespace parser
}       //  end for namespace ltp

#endif  //  end for __DEBUG_H__
