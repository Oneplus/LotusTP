#include "extractor.h"
#include "options.h"
#include "settings.h"

#define LEN(x)      (x.size())
#define LAST(x)     ((x)[(x).size()-1])
#define FIRST(x)    ((x)[0])

#define PUSH(x) do {\
    cache.push_back((x)); \
}while(0);

#define PUSH_DIST(x) do { \
    if (feat_opt.use_distance_in_features) { \
        (x).append(dist); \
        PUSH(x);    \
    } \
} while (0);

namespace ltp {
namespace parser {

// function of GET direction
void Extractor::__GET_DIRECTION(int head_id, int child_id, string& direction) {
    if (head_id == 0) {
        direction = "L#R";
    } else {
        direction = (head_id > child_id ? "L" : "R");
    }
}

void Extractor::__GET_DISTANCE_1_2_36_7(int head_id, int child_id, string& distance) {
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

const string POSUExtractor::prefix = "PU-";

int POSUExtractor::extract1o(Instance * inst, int hid, vector<string> & cache) {
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

// ================================================================ //
// Dependency Features Extractor                                    //
//  feature templates is listed in `extractor.h`                    //
//  the DEPExtractor is a singleton, which only be construct once   //
//  during the life of the program.                                 //
// ================================================================ //

// Initialize the static member
DEPExtractor * DEPExtractor::instance_ = 0;
vector<Template *> DEPExtractor::templates;

// Constructor for DEPExtractor, initialize templates for dependency
// features. It's some kind of tedious coding work.
DEPExtractor::DEPExtractor() {
    templates.reserve(100);

    // basic dependency unigram feature group
    if (feat_opt.use_dependency_unigram) {
        templates.push_back(new Template("1={f-hid}-{p-hid}-{dir}"));
        templates.push_back(new Template("2={f-hid}-{dir}"));
        templates.push_back(new Template("3={p-hid}-{dir}"));
        templates.push_back(new Template("4={f-cid}-{p-cid}-{dir}"));
        templates.push_back(new Template("5={f-cid}-{dir}"));
        templates.push_back(new Template("6={p-cid}-{dir}"));

        if (feat_opt.use_distance_in_features) {
            templates.push_back(new Template("1={f-hid}-{p-hid}-{dir}-{dist}"));
            templates.push_back(new Template("2={f-hid}-{dir}-{dist}"));
            templates.push_back(new Template("3={p-hid}-{dir}-{dist}"));
            templates.push_back(new Template("4={f-cid}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("5={f-cid}-{dir}-{dist}"));
            templates.push_back(new Template("6={p-cid}-{dir}-{dist}"));
        }

    }   //  end for if use dependency unigram

    if (feat_opt.use_dependency_unigram && feat_opt.use_lemma) {
        templates.push_back(new Template("77={l-hid}-{p-hid}-{dir}"));
        templates.push_back(new Template("78={l-hid}-{dir}"));
        templates.push_back(new Template("80={l-cid}-{p-cid}-{dir}"));
        templates.push_back(new Template("81={l-cid}-{dir}"));

        if (feat_opt.use_distance_in_features) {
            templates.push_back(new Template("77={l-hid}-{p-hid}-{dir}-{dist}"));
            templates.push_back(new Template("78={l-hid}-{dir}-{dist}"));
            // templates.push_back(new Template("80={l-cid}-{p-cid}-{dir}"));
            // templates.push_back(new Template("81={l-cid}-{dir}"));
        }   // end for if use distance in dependency features
    }   //  end for if use lemma

    if (feat_opt.use_dependency_unigram && feat_opt.use_coarse_postag) {
        templates.push_back(new Template("1C={f-hid}-{cp-hid}-{dir}"));
        templates.push_back(new Template("3C={cp-hid}-{dir}"));
        templates.push_back(new Template("4C={f-cid}-{cp-cid}-{dir}"));
        templates.push_back(new Template("6C={cp-cid}-{dir}"));

        if (feat_opt.use_distance_in_features) {
            templates.push_back(new Template("1C={f-hid}-{cp-hid}-{dir}-{dist}"));
            templates.push_back(new Template("3C={cp-hid}-{dir}-{dist}"));
            templates.push_back(new Template("4C={f-cid}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("6C={cp-cid}-{dir}-{dist}"));
        }
    }   //  end for use unigram and use coarse

    if (feat_opt.use_dependency_bigram) {
        templates.push_back(new Template("7={f-hid}-{p-hid}-{f-cid}-{p-cid}-{dir}"));
        templates.push_back(new Template("8={p-hid}-{f-cid}-{p-cid}-{dir}"));
        templates.push_back(new Template("9={f-hid}-{f-cid}-{p-cid}-{dir}"));
        templates.push_back(new Template("10={f-hid}-{p-hid}-{p-cid}-{dir}"));
        templates.push_back(new Template("11={f-hid}-{p-hid}-{f-cid}-{dir}"));
        templates.push_back(new Template("12={f-hid}-{f-cid}-{dir}"));
        templates.push_back(new Template("13={p-hid}-{p-cid}-{dir}"));
        templates.push_back(new Template("12?={f-hid}-{p-cid}-{dir}"));
        templates.push_back(new Template("13?={p-hid}-{f-cid}-{dir}"));

        if (feat_opt.use_distance_in_features) {
            // templates.push_back(new Template("7={f-hid}-{p-hid}-{f-cid}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("8={p-hid}-{f-cid}-{p-cid}-{dir}-{dist}"));
            // templates.push_back(new Template("9={f-hid}-{f-cid}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("10={f-hid}-{p-hid}-{p-cid}-{dir}-{dist}"));
            // templates.push_back(new Template("11={f-hid}-{p-hid}-{f-cid}-{dir}-{dist}"));
            templates.push_back(new Template("12={f-hid}-{f-cid}-{dir}-{dist}"));
            templates.push_back(new Template("13={p-hid}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("12?={f-hid}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("13?={p-hid}-{f-cid}-{dir}-{dist}"));
        }
    }   //  end for if use dependency bigram

    if (feat_opt.use_dependency_bigram && feat_opt.use_lemma) {
        templates.push_back(new Template("83={l-hid}-{p-hid}-{l-cid}-{p-cid}-{dir}"));
        templates.push_back(new Template("84={p-hid}-{l-cid}-{p-cid}-{dir}"));
        templates.push_back(new Template("85={l-hid}-{p-hid}-{p-cid}-{dir}"));
        templates.push_back(new Template("86={l-hid}-{p-hid}-{p-cid}-{dir}"));
        templates.push_back(new Template("87={l-hid}-{p-hid}-{l-cid}-{dir}"));

        if (feat_opt.use_distance_in_features) {
            // templates.push_back(new Template("83={l-hid}-{p-hid}-{l-cid}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("84={p-hid}-{l-cid}-{p-cid}-{dir}-{dist}"));
            // templates.push_back(new Template("85={l-hid}-{p-hid}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("86={l-hid}-{p-hid}-{p-cid}-{dir}-{dist}"));
            // templates.push_back(new Template("87={l-hid}-{p-hid}-{l-cid}-{dir}-{dist}"));
       }
    }   //  end for is use dependency bigram and use lemma

    if (feat_opt.use_dependency_bigram && feat_opt.use_coarse_postag) {
        templates.push_back(new Template("7C={f-hid}-{cp-hid}-{f-cid}-{cp-cid}-{dir}"));
        templates.push_back(new Template("8C={cp-hid}-{f-cid}-{cp-cid}-{dir}"));
        templates.push_back(new Template("9C={f-hid}-{f-cid}-{cp-cid}-{dir}"));
        templates.push_back(new Template("10C={f-hid}-{cp-hid}-{cp-cid}-{dir}"));
        templates.push_back(new Template("11C={f-hid}-{cp-hid}-{f-cid}-{dir}"));
        //templates.push_back(new Template("12C={f-hid}-{f-cid}-{dir}"));
        templates.push_back(new Template("13C={p-hid}-{cp-cid}-{dir}"));
        templates.push_back(new Template("12?C={f-hid}-c{p-cid}-{dir}"));
        templates.push_back(new Template("13?C={cp-hid}-{f-cid}-{dir}"));
        if (feat_opt.use_distance_in_features) {
            templates.push_back(new Template("7C={f-hid}-{cp-hid}-{f-cid}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("8C={cp-hid}-{f-cid}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("9C={f-hid}-{f-cid}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("10C={f-hid}-{cp-hid}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("11C={f-hid}-{cp-hid}-{f-cid}-{dir}-{dist}"));
            //templates.push_back(new Template("12C={f-hid}-{f-cid}-{dir}"));
            templates.push_back(new Template("13C={p-hid}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("12?C={f-hid}-c{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("13?C={cp-hid}-{f-cid}-{dir}-{dist}"));
        }
    }

    if (feat_opt.use_dependency_surrounding) {
        templates.push_back(new Template("14={p-hid}-{p-hid+1}-{p-cid}-{dir}"));
        templates.push_back(new Template("15={p-hid}-{p-cid-1}-{p-cid}-{dir}"));
        templates.push_back(new Template("16={p-hid}-{p-cid}-{p-cid+1}-{dir}"));
        templates.push_back(new Template("17={p-hid}-{p-hid+1}-{p-cid-1}-{p-cid}-{dir}"));
        templates.push_back(new Template("18={p-hid-1}-{p-hid+1}-{p-cid-1}-{p-cid}-{dir}"));
        templates.push_back(new Template("19={p-hid}-{p-hid+1}-{p-cid}-{p-cid+1}-{dir}"));
        templates.push_back(new Template("20={p-hid-1}-{p-hid}-{p-cid-1}-{p-cid}-{dir}"));
        templates.push_back(new Template("?20={p-hid-1}-{p-hid}-{p-hid+1}-{p-cid}-{dir}"));
        templates.push_back(new Template("?19={p-hid}-{p-cid-1}-{p-cid}-{p-cid+1}-{dir}"));

        if (feat_opt.use_distance_in_features) {
            templates.push_back(new Template("14={p-hid}-{p-hid+1}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("15={p-hid}-{p-cid-1}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("16={p-hid}-{p-cid}-{p-cid+1}-{dir}-{dist}"));
            templates.push_back(new Template("17={p-hid}-{p-hid+1}-{p-cid-1}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("18={p-hid-1}-{p-hid+1}-{p-cid-1}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("19={p-hid}-{p-hid+1}-{p-cid}-{p-cid+1}-{dir}-{dist}"));
            templates.push_back(new Template("20={p-hid-1}-{p-hid}-{p-cid-1}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("?20={p-hid-1}-{p-hid}-{p-hid+1}-{p-cid}-{dir}-{dist}"));
            templates.push_back(new Template("?19={p-hid}-{p-cid-1}-{p-cid}-{p-cid+1}-{dir}-{dist}"));
        }
    }

    if (feat_opt.use_dependency_surrounding && feat_opt.use_coarse_postag) {
        templates.push_back(new Template("14C={cp-hid}-{cp-hid+1}-{cp-cid}-{dir}"));
        templates.push_back(new Template("15C={cp-hid}-{cp-cid-1}-{cp-cid}-{dir}"));
        templates.push_back(new Template("16C={cp-hid}-{cp-cid}-{cp-cid+1}-{dir}"));
        templates.push_back(new Template("17C={cp-hid}-{cp-hid+1}-{cp-cid-1}-{cp-cid}-{dir}"));
        templates.push_back(new Template("18C={cp-hid-1}-{cp-hid+1}-{cp-cid-1}-{cp-cid}-{dir}"));
        templates.push_back(new Template("19C={cp-hid}-{cp-hid+1}-{cp-cid}-{cp-cid+1}-{dir}"));
        templates.push_back(new Template("20C={cp-hid-1}-{cp-hid}-{cp-hid+1}-{cp-cid}-{dir}"));
        templates.push_back(new Template("?20C={cp-hid-1}-{cp-hid}-{cp-hid+1}-{cp-cid}-{dir}"));
        templates.push_back(new Template("?19C={cp-hid}-{cp-cid-1}-{cp-cid}-{cp-cid+1}-{dir}"));

        if (feat_opt.use_distance_in_features) {
            templates.push_back(new Template("14C={cp-hid}-{cp-hid+1}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("15C={cp-hid}-{cp-cid-1}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("16C={cp-hid}-{cp-cid}-{cp-cid+1}-{dir}-{dist}"));
            templates.push_back(new Template("17C={cp-hid}-{cp-hid+1}-{cp-cid-1}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("18C={cp-hid-1}-{cp-hid+1}-{cp-cid-1}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("19C={cp-hid}-{cp-hid+1}-{cp-cid}-{cp-cid+1}-{dir}-{dist}"));
            templates.push_back(new Template("20C={cp-hid-1}-{cp-hid}-{cp-hid+1}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("?20={cp-hid-1}-{cp-hid}-{cp-hid+1}-{cp-cid}-{dir}-{dist}"));
            templates.push_back(new Template("?19={cp-hid}-{cp-cid-1}-{cp-cid}-{cp-cid+1}-{dir}-{dist}"));
        }
    }

    if (feat_opt.use_dependency_between) {
        templates.push_back(new Template("39={p-sid}-verb={verbcnt}-{p-lid}-{dir}"));
        templates.push_back(new Template("39={p-sid}-conj={conjcnt}-{p-lid}-{dir}"));
        templates.push_back(new Template("39={p-sid}-punc={punccnt}-{p-lid}-{dir}"));

        if (feat_opt.use_distance_in_features) {
            templates.push_back(new Template("39={p-sid}-verb={verbcnt}-{p-lid}-{dir}-{dist}"));
            templates.push_back(new Template("39={p-sid}-conj={conjcnt}-{p-lid}-{dir}-{dist}"));
            templates.push_back(new Template("39={p-sid}-punc={punccnt}-{p-lid}-{dir}-{dist}"));
        }

        // a dirty trick here, template 39 will generate several feature from on single
        // template, place this two template at last.
        templates.push_back(new Template("39={p-sid}-{p-bid}-{p-lid}-{dir}"));
        if (feat_opt.use_distance_in_features) {
            templates.push_back(new Template("39={p-sid}-{p-bid}-{p-lid}-{dir}-{dist}"));
        }
    }
}

DEPExtractor::~DEPExtractor() {
    for (int i = 0; i < templates.size(); ++ i) {
        delete templates[i];
    }
}

// accessment function, which return the singleton class
DEPExtractor * DEPExtractor::extractor() {
    if (0 == instance_) {
        instance_ = new DEPExtractor;
    }

    return instance_;
}

// get number of templates
int DEPExtractor::num_templates() {
    if (0 == instance_) {
        instance_ = new DEPExtractor;
    }

    return templates.size();
}

int DEPExtractor::extract2o(Instance * inst, int hid, int cid, vector< StringVec > & cache) {
    int len = inst->size();

    // if the postag count is not calculate, calculate the postag.
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

    Template::Data data;

    bool is_root = (hid == 0);

    data.set( "f-hid",   inst->forms[hid] );
    data.set( "f-cid",   inst->forms[cid] );
    data.set( "p-hid",   inst->postags[hid] );
    data.set( "p-cid",   inst->postags[cid] );
    data.set( "p-hid-1", ((hid <= 1) ? NONE_POSTAG : inst->postags[hid - 1]) );
    data.set( "p-hid+1", ((is_root || hid+1 >= len) ? NONE_POSTAG : inst->postags[hid + 1]) );
    data.set( "p-cid-1", ((cid <= 1) ? NONE_POSTAG : inst->postags[cid - 1]) );
    data.set( "p-cid+1", ((cid + 1 >= len) ? NONE_POSTAG : inst->postags[cid +1]) );
    data.set( "dir",     dir );
    data.set( "dist",    dist );

    int large = (hid > cid ? hid : cid);
    int small = (hid < cid ? hid : cid);
    data.set( "p-sid", inst->postags[small] );
    data.set( "p-lid", inst->postags[large] );

    int cnt;
    cnt = inst->verb_cnt[large-1] - inst->verb_cnt[small]; cnt = (cnt > 3 ? 3 : cnt);
    data.set( "verbcnt", to_str(cnt) );
    cnt = inst->conj_cnt[large-1] - inst->conj_cnt[small]; cnt = (cnt > 3 ? 3 : cnt);
    data.set( "conjcnt", to_str(cnt) );
    cnt = inst->punc_cnt[large-1] - inst->punc_cnt[small]; cnt = (cnt > 3 ? 3 : cnt);
    data.set( "punccnt", to_str(cnt) );

    feat.reserve(1024);

    int N = templates.size();
    int NN = (feat_opt.use_dependency_between ? 
            (feat_opt.use_distance_in_features ? N - 2 : N - 1) : N);

    for (int i = (is_root ? 3 : 0); i < NN; ++ i) {
        templates[i]->render(data, feat);
        cache[i].push_back(feat);
    }

    StringMap<int> pos_seen;
    for (int r = small + 1; r < large; ++ r) {
        if ( pos_seen.get(inst->postags[r].c_str()) == NULL ) {
            data.set( "p-bid", inst->postags[r] );
            for (int i = NN; i < N; ++ i) {
                templates[i]->render(data, feat);
                cache[i].push_back(feat);
            }

            pos_seen.set( inst->postags[r].c_str(), 1);
        }
    }

    return 0;
}

// ================================================================ //
// Sibling Features Extractor                                       //
//  feature templates is listed in `extractor.h`                    //
//  the SIBExtractor is a singleton, which only be construct once   //
//  during the life of the program.                                 //
// ================================================================ //

// Initialize the static member, 
SIBExtractor * SIBExtractor::instance_ = 0;
vector<Template *> SIBExtractor::templates;

// the constructor function.
// guarantee that it's a singleton.
SIBExtractor * SIBExtractor::extractor() {
    if (0 == instance_) {
        instance_ = new SIBExtractor;
    }

    return instance_;
}

SIBExtractor::SIBExtractor() {
    templates.reserve(100);

    if (feat_opt.use_sibling_basic) {
        templates.push_back(new Template("30={p-hid}-{p-sid}-{p-cid}-{dir}"));
        templates.push_back(new Template("31?={f-hid}-{p-sid}-{p-cid}-{dir}"));
        templates.push_back(new Template("32?={p-hid}-{f-sid}-{p-cid}-{dir}"));
        templates.push_back(new Template("33?={p-hid}-{p-sid}-{f-cid}-{dir}"));
        templates.push_back(new Template("32={p-cid}-{p-sid}-{dir}"));
        templates.push_back(new Template("36={f-cid}-{f-sid}-{dir}"));
        templates.push_back(new Template("37={f-cid}-{p-sid}-{dir}"));
        templates.push_back(new Template("38={p-cid}-{f-sid}-{dir}"));
    }

    if (feat_opt.use_sibling_basic && feat_opt.use_distance_in_features) {
        templates.push_back(new Template("30={p-hid}-{p-sid}-{p-cid}-{dir}-{dist}"));
        templates.push_back(new Template("31?={f-hid}-{p-sid}-{p-cid}-{dir}-{dist}"));
        templates.push_back(new Template("32?={p-hid}-{f-sid}-{p-cid}-{dir}-{dist}"));
        templates.push_back(new Template("33?={p-hid}-{p-sid}-{f-cid}-{dir}-{dist}"));
        templates.push_back(new Template("32={p-cid}-{p-sid}-{dir}-{dist}"));
        templates.push_back(new Template("36={f-cid}-{f-sid}-{dir}-{dist}"));
        templates.push_back(new Template("37={f-cid}-{p-sid}-{dir}-{dist}"));
        templates.push_back(new Template("38={p-cid}-{f-sid}-{dir}-{dist}"));
    }

    if (feat_opt.use_sibling_basic && model_opt.labeled) {
        templates.push_back(new Template("31={p-hid}-{p-sid}-{dir}"));
        templates.push_back(new Template("33={f-hid}-{f-sid}-{dir}"));
        templates.push_back(new Template("34={p-hid}-{f-sid}-{dir}"));
        templates.push_back(new Template("35={f-hid}-{p-sid}-{dir}"));
    }

    // if (feat_opt.use_sibling_basic && model_opt.labeled && feat_opt.use_distance_in_features) {
    if (feat_opt.use_sibling_basic) {
        templates.push_back(new Template("31={p-hid}-{p-sid}-{dir}-{dist}"));
        templates.push_back(new Template("33={f-hid}-{f-sid}-{dir}-{dist}"));
        templates.push_back(new Template("34={p-hid}-{f-sid}-{dir}-{dist}"));
        templates.push_back(new Template("35={f-hid}-{p-sid}-{dir}-{dist}"));
    }

    if (feat_opt.use_sibling_linear) {
        templates.push_back(new Template("66={p-cid}-{p-sid}-{p-sid+1}-{dir}"));
        templates.push_back(new Template("67={p-cid}-{p-sid-1}-{p-sid}-{dir}"));
        templates.push_back(new Template("68={p-cid}-{p-cid+1}-{p-sid}-{dir}"));
        templates.push_back(new Template("69={p-cid-1}-{p-cid}-{p-sid}-{dir}"));
        templates.push_back(new Template("70={p-cid-1}-{p-cid}-{p-sid}-{p-sid+1}-{dir}"));
        templates.push_back(new Template("71={p-cid-1}-{p-cid}-{p-sid-1}-{p-sid}-{dir}"));
        templates.push_back(new Template("72={p-cid}-{p-cid+1}-{p-sid}-{p-sid+1}-{dir}"));
        templates.push_back(new Template("73={p-cid}-{p-cid+1}-{p-sid-1}-{p-sid}-{dir}"));
    }

    /*if (feat_opt.use_sibling_linear && feat_opt.use_distance_in_features) {
        templates.push_back(new Template("66={p-cid}-{p-sid}-{p-sid+1}-{dir}-{dist}"));
        templates.push_back(new Template("67={p-cid}-{p-sid-1}-{p-sid}-{dir}-{dist}"));
        templates.push_back(new Template("68={p-cid}-{p-cid+1}-{p-sid}-{dir}-{dist}"));
        templates.push_back(new Template("69={p-cid-1}-{p-cid}-{p-sid}-{dir}-{dist}"));
        templates.push_back(new Template("70={p-cid-1}-{p-cid}-{p-sid}-{p-sid+1}-{dir}-{dist}"));
        templates.push_back(new Template("71={p-cid-1}-{p-cid}-{p-sid-1}-{p-sid}-{dir}-{dist}"));
        templates.push_back(new Template("72={p-cid}-{p-cid+1}-{p-sid}-{p-sid+1}-{dir}-{dist}"));
        templates.push_back(new Template("73={p-cid}-{p-cid+1}-{p-sid-1}-{p-sid}-{dir}-{dist}"));
    }*/


    if (feat_opt.use_sibling_linear && model_opt.labeled) {
        templates.push_back(new Template("58={p-hid}-{p-sid}-{p-sid+1}-{dir}"));
        templates.push_back(new Template("59={p-hid}-{p-sid-1}-{p-sid}-{dir}"));
        templates.push_back(new Template("60={p-hid}-{p-hid+1}-{p-sid}-{dir}"));
        templates.push_back(new Template("61={p-hid-1}-{p-hid}-{p-sid}-{dir}"));
        templates.push_back(new Template("62={p-hid-1}-{p-hid}-{p-sid}-{p-sid+1}-{dir}"));
        templates.push_back(new Template("63={p-hid-1}-{p-hid}-{p-sid-1}-{p-sid}-{dir}"));
        templates.push_back(new Template("64={p-hid}-{p-hid+1}-{p-sid}-{p-sid+1}-{dir}"));
        templates.push_back(new Template("65={p-hid}-{p-hid+1}-{p-sid-1}-{p-sid}-{dir}"));
    }   //  end for use sibling linear

    /*if (feat_opt.use_sibling_linear && model_opt.labeled && feat_opt.use_distance_in_features) {
        templates.push_back(new Template("58={p-hid}-{p-sid}-{p-sid+1}-{dir}-{dist}"));
        templates.push_back(new Template("59={p-hid}-{p-sid-1}-{p-sid}-{dir}-{dist}"));
        templates.push_back(new Template("60={p-hid}-{p-hid+1}-{p-sid}-{dir}-{dist}"));
        templates.push_back(new Template("61={p-hid-1}-{p-hid}-{p-sid}-{dir}-{dist}"));
        templates.push_back(new Template("62={p-hid-1}-{p-hid}-{p-sid}-{p-sid+1}-{dir}-{dist}"));
        templates.push_back(new Template("63={p-hid-1}-{p-hid}-{p-sid-1}-{p-sid}-{dir}-{dist}"));
        templates.push_back(new Template("64={p-hid}-{p-hid+1}-{p-sid}-{p-sid+1}-{dir}-{dist}"));
        templates.push_back(new Template("65={p-hid}-{p-hid+1}-{p-sid-1}-{p-sid}-{dir}-{dist}"));
    }*/   //  end for use sibling linear
}

SIBExtractor::~SIBExtractor() {
    for (int i = 0; i < templates.size(); ++ i) {
        delete templates[i];
    }
}

int SIBExtractor::num_templates() {
    if (0 == instance_) {
        instance_ = new SIBExtractor;
    }

    return templates.size();
}

int SIBExtractor::extract3o(Instance * inst, int hid, int cid, int sid, vector< StringVec > & cache) {
    int len = inst->size();
    bool first_child = (hid == sid);
    bool last_child = (cid == sid);

    string dir, dist, feat;

    __GET_DIRECTION(hid, cid, dir);
    __GET_DISTANCE_1_2_36_7(hid, cid, dist);

    if (first_child) dir = "#" + dir;
    else if (last_child) dir = dir + "#";

    Template::Data data;

    bool is_root = (hid == 0);

    data.set( "f-hid",   inst->forms[hid] );
    data.set( "f-sid",   ((first_child || last_child) ? NONE_FORM : inst->forms[sid]));
    data.set( "f-cid",   inst->forms[cid] );
    data.set( "p-hid",   inst->postags[hid] );
    data.set( "p-sid",   ((first_child || last_child) ? NONE_POSTAG : inst->postags[sid]));
    data.set( "p-cid",   inst->postags[cid] );
    data.set( "p-hid-1", ((hid <= 1) ? NONE_POSTAG : inst->postags[hid - 1]) );
    data.set( "p-hid+1", ((is_root || hid+1 >= len) ? NONE_POSTAG : inst->postags[hid + 1]) );
    data.set( "p-cid-1", ((cid <= 1) ? NONE_POSTAG : inst->postags[cid - 1]) );
    data.set( "p-cid+1", ((cid + 1 >= len) ? NONE_POSTAG : inst->postags[cid +1]) );
    data.set( "p-sid-1", ((first_child || last_child || sid <= 1) ? NONE_POSTAG : inst->postags[sid-1]));
    data.set( "p-sid+1", ((first_child || last_child || sid+1 >= len) ? NONE_POSTAG : inst->postags[sid+1]));
    data.set( "dir",     dir );
    data.set( "dist",    dist );

    feat.reserve(1024);

    for (int i = 0; i < templates.size(); ++ i) {
        templates[i]->render(data, feat);
        cache[i].push_back(feat);
    }
}

}       //  end for namespace parser
}       //  end for namespace ltp

#undef  LEN
#undef  LAST
#undef  PUSH
#undef  PUSH_DIST

