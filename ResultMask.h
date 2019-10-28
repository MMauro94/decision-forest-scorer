//
// Created by molin on 28/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_RESULTMASK_H
#define FOREST_TREE_EVALUATOR_RESULTMASK_H


#include <vector>
#include <stdint-gcc.h>
#include "Tree.h"
#include "Epitome.h"

class ResultMask {
private:
    std::shared_ptr<Forest> forest;
    std::vector<std::vector<uint8_t>> masks;
public:
    explicit ResultMask(std::shared_ptr<Forest> forest);

    void applyMask(const Epitome &mask, const int treeIndex) {
        mask.performAnd(this->masks[treeIndex]);
    }

    [[nodiscard]] double computeScore() const {
        double score = 0;
        for (unsigned long i = 0; i < this->forest->trees.size(); i++) {
            auto leafIndex = firstOne(this->masks[i]);
            auto tree = this->forest->trees[i];
            score += tree.scoreByLeafIndex(leafIndex);
        }
        return score;
    }

private:

    static unsigned int firstOne(const std::vector<uint8_t> &vector) {
        int ret = 0;
        int i = 0;
        while (vector[i] == 0) {
            i++;
            ret += 8;
        }
        int j = 0;
        while ((vector[i] & (1u << (8u - j - 1u))) == 0) {
            j++;
            ret++;
        }
        return ret;
    }

};

#endif //FOREST_TREE_EVALUATOR_RESULTMASK_H
