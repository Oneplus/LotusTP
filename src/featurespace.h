#ifndef __FEATURE_SPACE_H__
#define __FEATURE_SPACE_H__

#include <iostream>
#include <vector>

#include "instance.h"
#include "collections.h"

namespace ltp {
namespace parser {

/*
 * Feature Space Class, used to process features
 *
 */
class FeatureSpace {
public:
    enum { DEP, SIB, GRD, GRDSIB, POSU, POSB };

public:
    /*
     * Constructor of feature space, construct feature space with
     * instances. Number of dependency relations is given when 
     * constructing the feature space.
     *
     *  @param[in]  _num_deprels    the number of dependency relations
     */
    FeatureSpace() : _num_deprels(1) {
        // set all the group to be empty
        for (int i = 0; i < NUM_FEATURE_GROUPS; ++ i) {
            groups[i]  = 0;
            offsets[i] = 0;
        }
    }

    ~FeatureSpace() {
        for (int i = 0; i < NUM_FEATURE_GROUPS; ++ i) {
            if (groups[i]) {
                delete groups[i];
            }
        }
    }

    /*
     * Build feature space from the instances
     *
     *  @param[in]  instances   the instances
     */
    int build_feature_space( int num_deprels, const std::vector<Instance *> & instances);


    /*
     * Retrieve the feature string and maintain its index in the
     * feature space.
     *
     *  @param[in]  gid,    the group index
     *  @parma[in]  tid,    the template index
     *  @param[in]  key,    the key
     *  @param[in]  create, if true, a new key will be created.
     *  @return     int     index of the key, -1 if not exist.
     */
    int retrieve(int gid, int tid, const char * key, bool create);


    /*
     * Maintain the index of this key with certain label, for the
     * labeled case, lid is automically set as zero.
     *
     *  @param[in]  gid,    the group index
     *  @param[in]  tid,    the template index
     *  @param[in]  key,    the key
     *  @param[in]  lid,    the label index, default as zero
     *  @return     int     the index of the (key, label)
     */
    int index(int gid, int tid, const char * key, int lid = 0);

    int dim();

    int num_features();
private:
    const static int NUM_FEATURE_GROUPS = 6;

    int _offset;
    int _num_features;
    int _num_deprels;

    int offsets[NUM_FEATURE_GROUPS]; /*< offset of group */
    DictionaryCollections * groups[NUM_FEATURE_GROUPS];

private:
    /*
     * Allocate the feature groups according the options
     *
     *  @return     int         number of actived dictinary group
     */
    int allocate_dictionary_groups();
};

}   // end for namespace parser
}   // end for namespace ltp

#endif  //  end for __FEATURE_SPACE_H__

