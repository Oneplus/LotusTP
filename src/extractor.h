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
    feat_str_cache.push_back((x)); \
}while(0);

class Extractor {
public:
    Extractor(bool _train = false) : train(_train) {}
    virtual ~Extractor() {}

    virtual int extract(Instance * inst) = 0;

    void off() {
        train = false;
    }

    void on() {
        train = true;
    }

protected:
    Dictionary *    dict;
    bool            train;
};  //  end for class Extractor


class SiblingExtractor : public Extractor {
public:
    SiblingExtractor(DictionaryCollections * collections) {
        dict = collections->create_dict("sibling");
    }

    int extract(Instance * inst) {
    }
};  //  end for SiblingExtractor

/*
 * POSTag unigram features extractor
 */
class POSUnigramExtractor : public Extractor {
public:
    POSUnigramExtractor(DictionaryCollections& collections,
            bool _train) : 
        Extractor(_train), 
        prefix("PU-") {
        dict = collections.create_dict("pos-unigram");
    }

    int extract(Instance * inst) {
        // inst->fv_upos.resize(len);
        int len = inst->size();
        inst->postag_unigram_features.resize(len);
        inst->postag_unigram_scores.resize(len);

        for (int i = 0; i < len; ++ i) {
            const string &form = inst->forms[i];
            const string &left_form = (i > 0  ? inst->forms[i - 1] : NONE_FORM);
            const string &right_form =  (i + 1 < len ? inst->forms[i + 1] : NONE_FORM);

            string          feat;
            vector<string>  feat_str_cache;
            vector<int>     feat_idx_cache;

            feat = prefix + "0=" + form;            PUSH(feat);
            feat = prefix + "W-1=" + left_form;     PUSH(feat);
            feat = prefix + "W-1=" + right_form;    PUSH(feat);

            if (feat_opt.use_postag_chars) {
                const vector<string> &chars = inst->chars[i];
                const string &left_char  = (i > 0 ? LAST(inst->chars[i-1]) : NONE_FORM);
                const string &right_char = (i + 1 < len ? FIRST(inst->chars[i+1]) : NONE_FORM);

                string first_char = FIRST(chars);
                string last_char  = LAST(chars);

                int LL = LEN(chars);

                if (LEN(chars) == 1) {
                    first_char  += "#1";
                    last_char   += "#1";
                }

                feat = prefix + "1=" + form + FSEP + left_char;     PUSH(feat);
                feat = prefix + "2=" + form + FSEP + right_char;    PUSH(feat);

                if (LEN(chars) == 1) {
                    feat = prefix + "3=" + left_char + FSEP + form + FSEP + right_char;
                    PUSH(feat);
                }

                feat = prefix + "4=" + first_char;  PUSH(feat);
                feat = prefix + "5=" + last_char;   PUSH(feat);

                if (LEN(chars) > 2) {
                    for (int j = 2; j < LL - 1; ++ j) {
                        feat = prefix + "6=" + chars[j];                            PUSH(feat);
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

            for (int j = 0; j < LEN(feat_str_cache); ++ j) {
                int idx = dict->retrieve(feat_str_cache[j].c_str(), train);
                if (idx >= 0) {
                    feat_idx_cache.push_back(idx);
                }
            }

            int num_feat = LEN(feat_idx_cache);
            inst->postag_unigram_features[i] = new FeatureVector;
            inst->postag_unigram_features[i]->n   = num_feat;
            inst->postag_unigram_features[i]->idx = 0;
            inst->postag_unigram_features[i]->val = 0;

            if (num_feat > 0) {
                inst->postag_unigram_features[i]->idx = new int[num_feat];
            }
        }   //  end for for (int i = 0; i < len; ++ i)
    }   //  end for int extract
private:
    const string prefix;
};  //  end for class PosUnigramExtractor

/*
 * POSTag bigram feature extractor
 */
class POSBigramExtractor : public Extractor {
public:
    POSBigramExtractor(DictionaryCollections& collections,
            bool _train) : 
        Extractor(_train),
        prefix("") {
        dict = collections.create_dict("pos-bigram");
    }

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
    DependencyExtractor(DictionaryCollections& collections,
            bool _train) :
        Extractor(_train), 
        prefix("") {
        dict = collections.create_dict("dependency");
    }

    int extract(Instance * inst) {
        int len = inst->size();
        inst->dependency_features.resize(len, len);
        inst->dependency_scores.resize(len, len);

        vector<int> verb_cnt(len, 0);
        vector<int> conj_cnt(len, 0);
        vector<int> punc_cnt(len, 0);

        for (int h = 1; h < len; ++ h) {
            verb_cnt[h] = verb_cnt[h - 1];
            conj_cnt[h] = conj_cnt[h - 1];
            punc_cnt[h] = punc_cnt[h - 1];

            const string & tag = inst->postags[h];

            if(tag == "v" || tag == "V") {
                ++ verb_cnt[h];
            } else if(tag == "wp" || tag == "WP" || tag == "Punc" ||    tag == "PU" || tag == "," || tag == ":") {
                ++ punc_cnt[h];
            } else if( tag == "Conj" || tag == "CC" || tag == "cc" || tag == "c") {
                ++ conj_cnt[h];
            }
        }

        for (int h = 0; h < len; ++ h) {
            for (int m = 1; m < len; ++ m) {
                inst->dependency_features[h][m] = NULL;
                inst->dependency_scores[h][m] = DOUBLE_NEG_INF;

                if (h == m) {
                    continue;
                }
                /*if (inst->heads[m] != h) {
                    inst->dependency_features[h][m] = new FeatureVector;
                    inst->dependency_features[h][m]->n = 0;
                    continue;
                }*/

                vector<string>  feat_str_cache;
                vector<int>     feat_idx_cache;

                int head_id = h;
                int child_id = m;

                string dir, dist, feat;

                __GET_DIRECTION(head_id, child_id, dir);
                __GET_DISTANCE_1_2_36_7(head_id, child_id, dist);

                bool is_root = (head_id == 0);
                const string & fh = inst->forms[head_id];
                const string & fc = inst->forms[child_id];
                const string & lh = feat_opt.use_lemma ? inst->lemmas[head_id] : NONE_LEMMA;
                const string & lc = feat_opt.use_lemma ? inst->lemmas[child_id] : NONE_LEMMA;
                const string & ph = inst->postags[head_id];     // postag head
                const string & pc = inst->postags[child_id];    // postag child
                string cph, cpc;    // coarse postag head, coarse post child

                if (feat_opt.use_coarse_postag) {
                    cph = ((ph != PRP) 
                            && (ph != PRP2)
                            && (ph.length() > 2)) ? ph.substr(0, 2) : ph;
                    cpc = ((pc != PRP)
                            && (pc != PRP2)
                            && (pc.length() > 2)) ? pc.substr(0, 2) : pc;
                }

#define PUSH_DIST(x) do { \
    if (feat_opt.use_distance_in_dependency_features) { \
        (x) = ((x) + dist); \
        PUSH(x);    \
    } \
} while (0);

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
                    feat ="11=" + fh + FSEP + ph + FSEP + fc + FSEP + dir;  PUSH(feat); // PUSH_DIST(feat);
                    feat ="12=" + fh + FSEP + fc + FSEP + dir;              PUSH(feat); PUSH_DIST(feat);
                    feat ="13=" + ph + FSEP + pc + FSEP + dir;              PUSH(feat); PUSH_DIST(feat);
                    feat="12?=" + fh + FSEP + pc + FSEP + dir;              PUSH(feat); PUSH_DIST(feat);
                    feat="13?=" + ph + FSEP + fc + FSEP + dir;              PUSH(feat); PUSH_DIST(feat);

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
                    const string &phL1 = (head_id <= 1) ? NONE_POSTAG : inst->postags[head_id - 1];
                    const string &phR1 = (is_root || head_id+1 >= inst->size()) ? NONE_POSTAG : inst->postags[head_id + 1];
                    const string &pcL1 = (child_id <= 1) ? NONE_POSTAG : inst->postags[child_id - 1];
                    const string &pcR1 = (child_id + 1 >= inst->size()) ? NONE_POSTAG : inst->postags[child_id +1];

                    feat = "14=" + ph + FSEP + phR1 + FSEP + pc + FSEP + dir;                   PUSH(feat); PUSH_DIST(feat);
                    feat = "15=" + ph + FSEP + pcL1 + FSEP + pc + FSEP + dir;                   PUSH(feat); PUSH_DIST(feat);
                    feat = "16=" + ph + FSEP + pc + FSEP + pcR1 + FSEP + dir;                   PUSH(feat); PUSH_DIST(feat);
                    feat = "17=" + ph + FSEP + phR1 + FSEP + pcL1 + FSEP + pc + FSEP + dir;     PUSH(feat); PUSH_DIST(feat);
                    feat = "18=" + phL1 + FSEP + phR1 + FSEP + pcL1 + FSEP + pc + FSEP + dir;   PUSH(feat); PUSH_DIST(feat);
                    feat = "19=" + ph + FSEP + phR1 + FSEP + pc + FSEP + pcR1 + FSEP + dir;     PUSH(feat); PUSH_DIST(feat);
                    feat = "20=" + phL1 + FSEP + ph + FSEP + pcL1 + FSEP + pc + FSEP + dir;     PUSH(feat); PUSH_DIST(feat);
                    feat= "?20=" + phL1 + FSEP + ph + FSEP + phR1 + FSEP + pc + FSEP + dir;     PUSH(feat); PUSH_DIST(feat);
                    feat= "?19=" + ph + FSEP + pcL1 + FSEP + pc + FSEP + pcR1 + FSEP + dir;     PUSH(feat); PUSH_DIST(feat);

                    if (feat_opt.use_coarse_postag) {
                        const string &cphL1 = (phL1 != PRP && phL1 != PRP2 && phL1.length() > 2) ? phL1.substr(0,2) : phL1;
                        const string &cphR1 = (phR1 != PRP && phR1 != PRP2 && phR1.length() > 2) ? phR1.substr(0,2) : phR1;
                        const string &cpcL1 = (pcL1 != PRP && pcL1 != PRP2 && pcL1.length() > 2) ? pcL1.substr(0,2) : pcL1;
                        const string &cpcR1 = (pcR1 != PRP && pcR1 != PRP2 && pcR1.length() > 2) ? pcR1.substr(0,2) : pcR1;

                        feat = "14C=" + cph + FSEP + cphR1 + FSEP + cpc + FSEP + dir;                   PUSH(feat); PUSH_DIST(feat);
                        feat = "15C=" + cph + FSEP + cpcL1 + FSEP + cpc + FSEP + dir;                   PUSH(feat); PUSH_DIST(feat);
                        feat = "16C=" + cph + FSEP + cpc + FSEP + cpcR1 + FSEP + dir;                   PUSH(feat); PUSH_DIST(feat);
                        feat = "17C=" + cph + FSEP + cphR1 + FSEP + cpcL1 + FSEP + cpc + FSEP + dir;    PUSH(feat); PUSH_DIST(feat);
                        feat = "18C=" + cphL1 + FSEP + cphR1 + FSEP + cpcL1 + FSEP + cpc + FSEP + dir;  PUSH(feat); PUSH_DIST(feat);
                        feat = "19C=" + cph + FSEP + cphR1 + FSEP + cpc + FSEP + cpcR1 + FSEP + dir;     PUSH(feat); PUSH_DIST(feat);
                        feat = "20C=" + cphL1 + FSEP + cph + FSEP + cpcL1 + FSEP + cpc + FSEP + dir;    PUSH(feat); PUSH_DIST(feat);
                        feat= "?20C=" + cphL1 + FSEP + cph + FSEP + cphR1 + FSEP + cpc + FSEP + dir;    PUSH(feat); PUSH_DIST(feat);
                        feat= "?19C=" + cph + FSEP + cpcL1 + FSEP + cpc + FSEP + cpcR1 + FSEP + dir;    PUSH(feat); PUSH_DIST(feat);
                    }
                }   //  end for if (feat_opt.use_dependency_surrounding)

                if (feat_opt.use_dependency_between) {
                    int large = (head_id > child_id ? head_id : child_id);
                    int small = (head_id < child_id ? head_id : child_id);

                    const string & ps = inst->postags[small];
                    const string & pl = inst->postags[large];

                    for (int r = small + 1; r < large; ++ r) {
                        const string &pb = inst->postags[r];
                        paste(feat, "39=", ps, FSEP, pb, FSEP, pl, FSEP, dir); PUSH(feat); PUSH_DIST(feat);
                    }
                    int cnt;
                    cnt = (verb_cnt[large-1]-verb_cnt[small] < 3 ? verb_cnt[large-1]-verb_cnt[small] : 3);
                    paste(feat,"39=",ps,FSEP,"verb=",to_str(cnt),FSEP,pl,FSEP,dir); PUSH(feat); PUSH_DIST(feat);

                    cnt = (conj_cnt[large-1]-conj_cnt[small] < 3 ? conj_cnt[large-1]-conj_cnt[small] : 3);
                    paste(feat,"39=",ps,FSEP,"conj=",to_str(cnt),FSEP,pl,FSEP,dir); PUSH(feat); PUSH_DIST(feat);

                    cnt = (punc_cnt[large-1]-punc_cnt[small] < 3 ? punc_cnt[large-1]-punc_cnt[small] : 3);
                    paste(feat,"39=",ps,FSEP,"punc=",to_str(cnt),FSEP,pl,FSEP,dir); PUSH(feat); PUSH_DIST(feat);
                }   // end for if (feat_opt.use_dependency_between)

                for (int j = 0; j < LEN(feat_str_cache); ++ j) {
                    int idx = dict->retrieve(feat_str_cache[j].c_str(), 
                            (train && inst->heads[m] == h));
                    // std::cerr << "+" << feat_str_cache[j] << " " << idx << std::endl;
                    if (idx >= 0) {
                        feat_idx_cache.push_back(idx);
                    }
                }

                int num_feat = LEN(feat_idx_cache);
                inst->dependency_features[h][m]= new FeatureVector;
                inst->dependency_features[h][m]->n   = num_feat;
                inst->dependency_features[h][m]->idx = 0;
                inst->dependency_features[h][m]->val = 0;

                if (num_feat > 0) {
                    inst->dependency_features[h][m]->idx = new int[num_feat];
                    for (int j = 0; j < num_feat; ++ j) {
                        // std::cerr << "-" << feat_idx_cache[j] << std::endl;
                        inst->dependency_features[h][m]->idx[j] = feat_idx_cache[j];
                    }

                }
#undef  PUSH_DIST
            }
        }
    }

private:
    string          prefix;

private:
    void __GET_DIRECTION(int head_id, int child_id, string& direction) {
        if (head_id == 0) {
            direction = "L#R";
        } else {
            direction = (head_id > child_id ? "L" : "R");
        }
    }

    void __GET_DISTANCE_1_2_36_7(int head_id, int child_id, string& distance) {
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

}   //  end for namespace parser
}   //  end for namespace ltp

#endif  //  end for ifndef __EXTRACTOR_H__
