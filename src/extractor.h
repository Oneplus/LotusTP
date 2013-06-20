#ifndef __EXTRACTOR_H__
#define __EXTRACTOR_H__

#include <sstream>
#include "options.h"
#include "settings.h"
#include "collections.h"

#include "strutils.hpp"
#include "strpaste.hpp"

#include "debug.h"

namespace ltp {
namespace parser {

using namespace ltp::strutils;

#define LEN(x)      (x.size())
#define LAST(x)     ((x)[(x).size()-1])
#define FIRST(x)    ((x)[0])

#define PUSH(x) do {\
    cache.push_back((x)); \
}while(0);

#define PUSH_DIST(x) do { \
    if (feat_opt.use_distance_in_dependency_features) { \
        (x).append(dist); \
        PUSH(x);    \
    } \
} while (0);

class Extractor {
public:
    Extractor() {}
    virtual ~Extractor() {}

    // virtual static int extract1o(Instance * inst, int hid, vector<string> & cache) {}
    // virtual static int extract2o(Instance * inst, int hid, int cid, vector<string> & cache) {}
    // virtual static int extract3o(Instance * inst, int hid, int cid, int gid, vector<string> & cache) {}
};  //  end for class Extractor


class SiblingExtractor : public Extractor {
public:
    SiblingExtractor() {
    }

    int extract1o(Instance * inst, int hid, vector<string> & cache) {
    }

    int extract2o(Instance * inst, int hid, int cid, vector<string> & cache) {
    }

    int extract3o(Instance * inst, int hid, int cid, int gid, vector<string> & cache) {
    }
};  //  end for SiblingExtractor

/*
 * POSTag unigram features extractor
 */
class POSUnigramExtractor : public Extractor {
public:
    POSUnigramExtractor() : prefix("PU-") { }

    int extract1o(Instance * inst, int hid, vector<string> & cache) {
        // inst->fv_upos.resize(len);
        int len = inst->size();
        inst->postag_unigram_features.resize(len);
        inst->postag_unigram_scores.resize(len);

        const string &form = inst->forms[hid];
        const string &left_form = (hid > 0  ? inst->forms[hid - 1] : NONE_FORM);
        const string &right_form =  (hid + 1 < len ? inst->forms[hid + 1] : NONE_FORM);

        string          feat;

        paste(feat, prefix, "0=", form);            PUSH(feat);
        paste(feat, prefix, "W-1=", left_form);     PUSH(feat);
        paste(feat, prefix, "W-1=", right_form);    PUSH(feat);

        if (feat_opt.use_postag_chars) {
            const vector<string> &chars = inst->chars[hid];
            const string &left_char  = (hid > 0 ? LAST(inst->chars[hid - 1]) : NONE_FORM);
            const string &right_char = (hid + 1 < len ? FIRST(inst->chars[hid + 1]) : NONE_FORM);

            string first_char = FIRST(chars);
            string last_char  = LAST(chars);

            int LL = LEN(chars);

            if (LEN(chars) == 1) {
                first_char  += "#1";
                last_char   += "#1";
            }

            paste(feat, prefix, "1=", form, FSEP, left_char);   PUSH(feat);
            paste(feat, prefix, "2=", form, FSEP, right_char);  PUSH(feat);

            if (LEN(chars) == 1) {
                paste(feat, prefix, "3=", left_char, FSEP, form, FSEP, right_char); PUSH(feat);
            }

            paste(feat, prefix, "4=", first_char);  PUSH(feat);
            paste(feat, prefix, "5=", last_char);   PUSH(feat);

            if (LEN(chars) > 2) {
                for (int j = 2; j < LL - 1; ++ j) {
                    feat = prefix + "6=" + chars[j];                        PUSH(feat);
                    feat = prefix + "7=" + first_char + FSEP + chars[j];    PUSH(feat);
                    feat = prefix + "8=" + chars[j] + FSEP + last_char;     PUSH(feat);
                }
            }

            for (int j = 1; j < LL; ++ j) {
                if (chars[j] == chars[j - 1]) {
                    feat = prefix + "cc=" + chars[j];   PUSH(feat);
                }
            }

            string curprefix = "";
            string cursuffix = "";

            for (int j = 0; j <= 3 && j < LL; ++ j) {
                curprefix = curprefix + chars[j];
                cursuffix = chars[LL-j-1] + cursuffix;

                feat = prefix + "prefix=" + curprefix;  PUSH(feat);
                feat = prefix + "suffix=" + cursuffix;  PUSH(feat);
            }

            int length = (LL < 5 ? LL : 5); stringstream S; S << length;
            feat = prefix + "length=" + S.str();  PUSH(feat);
        }   //  end for if (use_pos_chars) 
    }   //  end for int extract
private:
    const string prefix;
};  //  end for class PosUnigramExtractor

/*
 * POSTag bigram feature extractor
 */
class POSBigramExtractor : public Extractor {
public:
    POSBigramExtractor() : prefix("") { }

    int extract(Instance * inst) {
    }   //  end for int extract
private:
    const string prefix;
};  //  end for class POSBigramExtractor

/*
 * Dependency feature extractor
 */
class DependencyExtractor : public Extractor {
public:
    DependencyExtractor() : prefix("") {}
    ~DependencyExtractor() {}

    int extract1o(Instance * inst, int hid, vector<string> & cache) {
    }

    static int extract2o(Instance * inst, int hid, int cid, vector<string> & cache) {
        int len = inst->size();

        if (inst->verb_cnt.size() == 0) {
            inst->verb_cnt.resize(len, 0);
            inst->conj_cnt.resize(len, 0);
            inst->punc_cnt.resize(len, 0);

            for (int h = 1; h < len; ++ h) {
                inst->verb_cnt[h] = inst->verb_cnt[h - 1];
                inst->conj_cnt[h] = inst->conj_cnt[h - 1];
                inst->punc_cnt[h] = inst->punc_cnt[h - 1];

                const string & tag = inst->postags[h];

                if(tag == "v" || tag == "V") {
                    ++ inst->verb_cnt[h];
                } else if(tag == "wp" || tag == "WP" || tag == "Punc" || tag == "PU" || tag == "," || tag == ":") {
                    ++ inst->punc_cnt[h];
                } else if( tag == "Conj" || tag == "CC" || tag == "cc" || tag == "c") {
                    ++ inst->conj_cnt[h];
                }
            }
        }

        string dir, dist, feat;

        __GET_DIRECTION(hid, cid, dir);
        __GET_DISTANCE_1_2_36_7(hid, cid, dist);

        bool is_root = (hid == 0);
        const string & fh = inst->forms[hid];
        const string & fc = inst->forms[cid];
        const string & lh = feat_opt.use_lemma ? inst->lemmas[hid] : NONE_LEMMA;
        const string & lc = feat_opt.use_lemma ? inst->lemmas[cid] : NONE_LEMMA;
        const string & ph = inst->postags[hid];     // postag head
        const string & pc = inst->postags[cid];    // postag child
        string cph, cpc;    // coarse postag head, coarse post child

        if (feat_opt.use_coarse_postag) {
            cph = ((ph != PRP) 
                    && (ph != PRP2)
                    && (ph.length() > 2)) ? ph.substr(0, 2) : ph;
            cpc = ((pc != PRP)
                    && (pc != PRP2)
                    && (pc.length() > 2)) ? pc.substr(0, 2) : pc;
        }

        feat.reserve(1024);
        if (feat_opt.use_dependency_unigram) {
            if (!is_root) {
                //
                paste(feat, "1=", fh, FSEP, ph, FSEP, dir);     PUSH(feat); PUSH_DIST(feat);
                paste(feat, "2=", fh, FSEP, dir);               PUSH(feat); PUSH_DIST(feat);
                paste(feat, "3=", ph, FSEP, dir);               PUSH(feat); PUSH_DIST(feat);

                if (feat_opt.use_lemma) {
                    feat = "77=" + lh + FSEP + ph + FSEP + dir; PUSH(feat); PUSH_DIST(feat);
                    feat = "78=" + lh + FSEP + dir;             PUSH(feat); PUSH_DIST(feat);
                }

                if (feat_opt.use_coarse_postag) {
                    feat = "1C=" + fh + FSEP + cph + FSEP + dir;PUSH(feat); PUSH_DIST(feat);
                    feat = "3C=" + cph + FSEP + dir;            PUSH(feat); PUSH_DIST(feat);
                }
            }

            if (true) { //  child
                paste(feat, "4=", fc, FSEP, pc, FSEP, dir);     PUSH(feat); PUSH_DIST(feat);
                paste(feat, "5=", fc, FSEP, dir);               PUSH(feat); PUSH_DIST(feat);
                paste(feat, "6=", pc, FSEP, dir);               PUSH(feat); PUSH_DIST(feat);

                if (feat_opt.use_lemma) {
                    feat = "80=" + lc + FSEP + pc + FSEP + dir; PUSH(feat); PUSH_DIST(feat);
                    feat = "81=" + lc + FSEP + dir;             PUSH(feat); PUSH_DIST(feat);
                }

                if (feat_opt.use_coarse_postag) {
                    feat = "4C=" + fc + FSEP + cpc + FSEP + dir;PUSH(feat); PUSH_DIST(feat);
                    feat = "6C=" + fc + FSEP + dir;             PUSH(feat); PUSH_DIST(feat);
                }
            }
        }   //  end for if (feat_opt.use_dependency_unigram)

        if (feat_opt.use_dependency_bigram) {
            paste(feat, "7=", fh, FSEP, ph, FSEP, fc,FSEP, pc, FSEP, dir); PUSH(feat); // PUSH_DIST(feat);
            paste(feat, "8=", ph, FSEP, fc, FSEP, pc, FSEP, dir);  PUSH(feat); PUSH_DIST(feat);
            paste(feat, "9=", fh, FSEP, fc, FSEP, pc, FSEP, dir);  PUSH(feat); // PUSH_DIST(feat);
            paste(feat, "10=",fh, FSEP, ph, FSEP, pc, FSEP, dir);  PUSH(feat); PUSH_DIST(feat);
            paste(feat, "11=",fh, FSEP, ph, FSEP, fc, FSEP, dir);  PUSH(feat); // PUSH_DIST(feat);
            paste(feat, "12=",fh, FSEP, fc, FSEP, dir);            PUSH(feat); PUSH_DIST(feat);
            paste(feat, "13=",ph, FSEP, pc, FSEP, dir);            PUSH(feat); PUSH_DIST(feat);
            paste(feat,"12?=",fh, FSEP, pc, FSEP, dir);            PUSH(feat); PUSH_DIST(feat);
            paste(feat,"13?=",ph, FSEP, fc, FSEP, dir);            PUSH(feat); PUSH_DIST(feat);

            if (feat_opt.use_lemma) {
                feat = "83=" + lh + FSEP + ph + FSEP + lc + FSEP + pc + FSEP + dir; PUSH(feat);
                feat = "84=" + ph + FSEP + lc + FSEP + pc + FSEP + dir; PUSH(feat); PUSH_DIST(feat);
                feat = "85=" + lh + FSEP + lc + FSEP + pc + FSEP + dir; PUSH(feat);
                feat = "86=" + lh + FSEP + ph + FSEP + pc + FSEP + dir; PUSH(feat); PUSH_DIST(feat);
                feat = "87=" + lh + FSEP + ph + FSEP + lc + FSEP + dir; PUSH(feat);
            }

            if (feat_opt.use_coarse_postag) {
                feat = "7C=" + fh + FSEP + cph + FSEP + fc + FSEP + cpc + FSEP + dir; PUSH(feat);
                feat = "8C=" + cph + FSEP + fc + FSEP + pc + FSEP + dir;  PUSH(feat); PUSH_DIST(feat);
                feat = "9C=" + fh + FSEP + fc + FSEP + cpc + FSEP + dir;  PUSH(feat);
                feat ="10C=" + fh + FSEP + cph + FSEP + cpc + FSEP + dir; PUSH(feat); PUSH_DIST(feat);
                feat ="11C=" + fh + FSEP + cph + FSEP + fc + FSEP + dir;  PUSH(feat);
                //feat ="12C=" + fh + FSEP + fc + FSEP + dir;               PUSH(feat); PUSH_DIST(feat);
                feat ="13C=" + cph + FSEP + cpc + FSEP + dir;             PUSH(feat); PUSH_DIST(feat);
                feat="12?C=" + fh + FSEP + cpc + FSEP + dir;              PUSH(feat); PUSH_DIST(feat);
                feat="13?C=" + cph + FSEP + fc + FSEP + dir;              PUSH(feat); PUSH_DIST(feat);
            }
        }   // end for if (feat_opt.use_dependency_bigram)

        if (feat_opt.use_dependency_surrounding) {
            const string &phL1 = (hid <= 1) ? NONE_POSTAG : inst->postags[hid - 1];
            const string &phR1 = (is_root || hid+1 >= len) ? NONE_POSTAG : inst->postags[hid + 1];
            const string &pcL1 = (cid <= 1) ? NONE_POSTAG : inst->postags[cid - 1];
            const string &pcR1 = (cid + 1 >= len) ? NONE_POSTAG : inst->postags[cid +1];

            paste(feat, "14=", ph, FSEP, phR1, FSEP, pc, FSEP, dir);                PUSH(feat); PUSH_DIST(feat);
            paste(feat, "15=", ph, FSEP, pcL1, FSEP, pc, FSEP, dir);                PUSH(feat); PUSH_DIST(feat);
            paste(feat, "16=", ph, FSEP, pc, FSEP, pcR1, FSEP, dir);                PUSH(feat); PUSH_DIST(feat);
            paste(feat, "17=", ph, FSEP, phR1, FSEP, pcL1, FSEP, pc, FSEP, dir);    PUSH(feat); PUSH_DIST(feat);
            paste(feat, "18=", phL1, FSEP, phR1, FSEP, pcL1, FSEP, pc, FSEP, dir);  PUSH(feat); PUSH_DIST(feat);
            paste(feat, "19=", ph, FSEP, phR1, FSEP, pc, FSEP, pcR1, FSEP, dir);    PUSH(feat); PUSH_DIST(feat);
            paste(feat, "20=", phL1, FSEP, ph, FSEP, pcL1, FSEP, pc, FSEP, dir);    PUSH(feat); PUSH_DIST(feat);
            paste(feat, "?20=", phL1, FSEP, ph, FSEP, phR1, FSEP, pc, FSEP, dir);   PUSH(feat); PUSH_DIST(feat);
            paste(feat, "?19=", ph, FSEP, pcL1, FSEP, pc, FSEP, pcR1, FSEP, dir);   PUSH(feat); PUSH_DIST(feat);

            if (feat_opt.use_coarse_postag) {
                const string &cphL1 =(phL1 != PRP && phL1 != PRP2 && phL1.length() > 2) ? phL1.substr(0,2) : phL1;
                const string &cphR1 =(phR1 != PRP && phR1 != PRP2 && phR1.length() > 2) ? phR1.substr(0,2) : phR1;
                const string &cpcL1 =(pcL1 != PRP && pcL1 != PRP2 && pcL1.length() > 2) ? pcL1.substr(0,2) : pcL1;
                const string &cpcR1 =(pcR1 != PRP && pcR1 != PRP2 && pcR1.length() > 2) ? pcR1.substr(0,2) : pcR1;
                paste(feat, "14C=", cph, FSEP, cphR1, FSEP, cpc, FSEP, dir);        PUSH(feat); PUSH_DIST(feat);
                paste(feat, "15C=", cph, FSEP, cpcL1, FSEP, cpc, FSEP, dir);        PUSH(feat); PUSH_DIST(feat);
                paste(feat, "16C=", cph, FSEP, cpc, FSEP, cpcR1, FSEP, dir);        PUSH(feat); PUSH_DIST(feat);
                paste(feat, "17C=", cph, FSEP, cphR1, FSEP, cpcL1, FSEP, cpc, FSEP, dir);  PUSH(feat); PUSH_DIST(feat);
                paste(feat, "18C=", cphL1, FSEP, cphR1, FSEP, cpcL1, FSEP, cpc, FSEP, dir); PUSH(feat); PUSH_DIST(feat);
                paste(feat, "19C=", cph, FSEP, cphR1, FSEP, cpc, FSEP, cpcR1, FSEP, dir);  PUSH(feat); PUSH_DIST(feat);
                paste(feat, "20C=", cphL1, FSEP, cph, FSEP, cpcL1, FSEP, cpc, FSEP, dir);  PUSH(feat); PUSH_DIST(feat);
                paste(feat, "?20C=", cphL1, FSEP, cph, FSEP, cphR1, FSEP, cpc, FSEP, dir); PUSH(feat); PUSH_DIST(feat);
                paste(feat, "?19C=", cph, FSEP, cpcL1, FSEP, cpc, FSEP, cpcR1, FSEP, dir); PUSH(feat); PUSH_DIST(feat);
            }
        }   //  end for if (feat_opt.use_dependency_surrounding)

        if (feat_opt.use_dependency_between) {
            int large = (hid > cid ? hid : cid);
            int small = (hid < cid ? hid : cid);
            const string & ps = inst->postags[small];
            const string & pl = inst->postags[large];

            StringMap<int> pos_seen;
            for (int r = small + 1; r < large; ++ r) {
                const string &pb = inst->postags[r];
                if ( pos_seen.get(pb.c_str()) == NULL ) {
                    paste(feat, "39=", ps, FSEP, pb, FSEP, pl, FSEP, dir); PUSH(feat); PUSH_DIST(feat);
                    pos_seen.set(pb.c_str(), 1);
                }
            }
            int cnt;

            cnt = inst->verb_cnt[large-1] - inst->verb_cnt[small];
            cnt = (cnt < 3 ? cnt : 3);
            paste(feat,"39=",ps,FSEP,"verb=",to_str(cnt),FSEP,pl,FSEP,dir); PUSH(feat); PUSH_DIST(feat);

            cnt = inst->conj_cnt[large-1] - inst->conj_cnt[small];
            cnt = (cnt < 3 ? cnt : 3);
            paste(feat,"39=",ps,FSEP,"conj=",to_str(cnt),FSEP,pl,FSEP,dir); PUSH(feat); PUSH_DIST(feat);

            cnt = inst->punc_cnt[large-1] - inst->punc_cnt[small];
            cnt = (cnt < 3 ? cnt : 3);
            paste(feat,"39=",ps,FSEP,"punc=",to_str(cnt),FSEP,pl,FSEP,dir); PUSH(feat); PUSH_DIST(feat);
        }   // end for if (feat_opt.use_dependency_between)

        return 0;
    }

    int extract3o(Instance * inst, int hid, int cid, int gid, vector<string> & cache) {
    }
private:
    string          prefix;

private:
    static void __GET_DIRECTION(int head_id, int child_id, string& direction) {
        if (head_id == 0) {
            direction = "L#R";
        } else {
            direction = (head_id > child_id ? "L" : "R");
        }
    }

    static void __GET_DISTANCE_1_2_36_7(int head_id, int child_id, string& distance) {
        int dist = (head_id > child_id ? head_id - child_id : child_id - head_id) ;

        if (dist < 3) {
            ostringstream S; S << dist; 
            distance = S.str();
        } else if (dist < 7) {
            distance = "<7";
        } else {
            distance = ">6";
        }
    }
};  //  end for class DependencyExtractor
#undef  LEN
#undef  LAST
#undef  PUSH
#undef  PUSH_DIST

}   //  end for namespace parser
}   //  end for namespace ltp

#endif  //  end for ifndef __EXTRACTOR_H__
