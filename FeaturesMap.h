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
    std::vector<std::shared_ptr<InternalNode>> nodes;
    std::vector<double> splittingThreshold;
    std::vector<Epitome> masks;
    std::vector<int> treeIndexed;
public:
    FeatureEvaluator(
            Forest &forest,
            int featureIndex
    ) : nodes(forest.getNodesForFeature(featureIndex)) {
        std::sort(this->nodes.begin(), this->nodes.end(), [](auto &a, auto &b) -> bool {
            return a->splittingThreshold < b->splittingThreshold;
        });


        for (auto &n : this->nodes) {
            printf("Found node %f for feature %d\n", n->splittingThreshold, featureIndex);
            splittingThreshold.push_back(n->splittingThreshold);
            treeIndexed.push_back(n->getTreeIndex());

            auto ep = masks.emplace_back(forest.trees[n->getTreeIndex()].countLeafsUntil(n), n->leftNode->numberOfLeafs());
            std::cout << ep << std::endl;
        }
    }
};

class FeaturesMap {

};


#endif //FOREST_TREE_EVALUATOR_FEATURESMAP_H
