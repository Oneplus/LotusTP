#ifndef __MODEL_H__
#define __MODEL_H__

#include "collections.h"
#include "parameters.h"
#include "options.h"

namespace ltp {
namespace parser {

class Model {
public:
    Model() : _dim(-1) {}
    ~Model() {}

    inline int num_deprels() {
        return deprels.size();
    }

    inline int num_postags() {
        return postags.size();
    }

    inline int num_features() {
        return collections.dim();
    }

    inline int dim() {
        if (_dim < 0) {
            _dim = 0;

            if (feat_opt.use_unlabeled_dependency) {
                _dim += collections.create_dict("dependency")->size();
            } else {
                _dim += (collections.create_dict("dependency")->size()) * num_deprels();
            }
        }

        return _dim;
    }

public:
    DictionaryCollections   collections;
    Parameters              param;

    LabelCollections        postags;
    LabelCollections        deprels;

    void save(ostream & out) {
        // write a signature
        char chunk[16] = {'l','g','d','p', 'j', 0};
        out.write(chunk, 16);
        unsigned int tmp;

        int off = out.tellp();

        unsigned basic_offset =  0;
        unsigned postag_offset = 0;
        unsigned deprels_offset = 0;
        unsigned feature_offset = 0;
        unsigned parameter_offset = 0;

        // write pseduo position
        write_uint(out, 0); //  basic offset
        write_uint(out, 0); //  postag offset
        write_uint(out, 0); //  deprels offset
        write_uint(out, 0); //  features offset
        write_uint(out, 0); //  parameters offset

        // model and feature information
        // labeled model
        basic_offset = out.tellp();
        tmp = model_opt.labeled;
        write_uint(out, tmp);

        // decode order
        strncpy(chunk, model_opt.decoder_name.c_str(), 16);
        out.write(chunk, 16);

        // use dependency
        tmp = feat_opt.use_dependency;
        write_uint(out, tmp);

        // use dependency unigram
        tmp = feat_opt.use_dependency_unigram;
        write_uint(out, tmp);

        // use dependency bigram
        tmp = feat_opt.use_dependency_bigram;
        write_uint(out, tmp);

        // use dependency surrounding
        tmp = feat_opt.use_dependency_surrounding;
        write_uint(out, tmp);

        // use dependency between
        tmp = feat_opt.use_dependency_between;
        write_uint(out, tmp);

        postag_offset = out.tellp();
        postags.dump(out);

        deprels_offset = out.tellp();
        deprels.dump(out);

        feature_offset = out.tellp();
        collections.dump(out);

        parameter_offset = out.tellp();
        param.dump(out);

        out.seekp(off);
        write_uint(out, basic_offset);
        // cout << out.tellp() << endl;
        write_uint(out, postag_offset);
        // cout << out.tellp() << endl;
        write_uint(out, deprels_offset);
        // cout << out.tellp() << endl;
        write_uint(out, feature_offset);
        // cout << out.tellp() << endl;
        write_uint(out, parameter_offset);

        // out.seekp(0, std::ios::end);
    }

    bool load(istream & in) {
        char chunk[16];
        in.read(chunk, 16);
        if (strcmp(chunk, "lgdpj")) {
            return false;
        }

        unsigned int basic_offset = read_uint(in);
        unsigned int postag_offset = read_uint(in);
        unsigned int deprels_offset = read_uint(in);
        unsigned int feature_offset = read_uint(in);
        unsigned int parameter_offset = read_uint(in);

        in.seekg(basic_offset);
        model_opt.labeled = (read_uint(in) == 1);

        // decode order
        in.read(chunk, 16);
        model_opt.decoder_name = chunk;

        // use dependency
        feat_opt.use_dependency = (read_uint(in) == 1);

        // use dependency unigram
        feat_opt.use_dependency_unigram = (read_uint(in) == 1);

        // use dependency bigram
        feat_opt.use_dependency_bigram = (read_uint(in) == 1);

        // use dependency surrounding
        feat_opt.use_dependency_surrounding = (read_uint(in) == 1);

        // use dependency between
        feat_opt.use_dependency_between = (read_uint(in) == 1);

        in.seekg(postag_offset);
        if (!postags.load(in)) {
            return false;
        }

        in.seekg(deprels_offset);
        if (!deprels.load(in)) {
            return false;
        }

        in.seekg(feature_offset);
        if (!collections.load(in)) {
            return false;
        }

        in.seekg(parameter_offset);
        if (!param.load(in)) {
            std::cout << "param failed" << std::endl;
            return false;
        }

        return true;
    }

private:
    int _dim;


private:
    void write_uint(ostream & out, unsigned int val) {
        out.write(reinterpret_cast<const char *>(&val), sizeof(unsigned int));
    }

    unsigned int read_uint(istream & in) {
        char p[4];
        in.read(reinterpret_cast<char*>(p), sizeof(unsigned int));
        return *reinterpret_cast<const unsigned int*>(p);
    }
};      //  end for class model

}       //  end for namespace parser
}       //  end for namespace ltp
#endif  //  end for __MODEL_H__
