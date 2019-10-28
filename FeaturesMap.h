//
// Created by Marco on 23/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_FEATURESMAP_H
#define FOREST_TREE_EVALUATOR_FEATURESMAP_H

#include <vector>
#include <algorithm>
#include "Tree.h"
#include "Epitome.h"

class FeatureEvaluator {
private:
    std::vector<double> splittingThreshold;
    std::vector<Epitome> masks;
    std::vector<int> treeIndexed;
public:
    FeatureEvaluator(
            Forest &forest,
            int featureIndex
    ) {
        auto nodes = forest.getNodesForFeature(featureIndex);
        std::sort(nodes.begin(), nodes.end(), [](auto &a, auto &b) -> bool {
            return a->splittingThreshold < b->splittingThreshold;
        });


        for (auto &n : nodes) {
            splittingThreshold.push_back(n->splittingThreshold);
            treeIndexed.push_back(n->getTreeIndex());

            masks.emplace_back(forest.trees[n->getTreeIndex()].countLeafsUntil(n), n->leftNode->numberOfLeafs());
        }
    }
};

class FeaturesMap {

};


#endif //FOREST_TREE_EVALUATOR_FEATURESMAP_H
