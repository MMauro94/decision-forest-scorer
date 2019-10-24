//
// Created by Marco on 23/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_FEATURESMAP_H
#define FOREST_TREE_EVALUATOR_FEATURESMAP_H

#include <vector>
#include <algorithm>
#include "Tree.h"

class FeatureEvaluator {
	private:
		std::vector<std::shared_ptr<InternalNode>> nodes;
		std::vector<double> splittingThreshold;
		std::vector<int> masks;
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
				masks.push_back(0);//TODO
			}
		}
};

class FeaturesMap {

};


#endif //FOREST_TREE_EVALUATOR_FEATURESMAP_H
