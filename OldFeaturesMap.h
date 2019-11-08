//
// Created by Marco on 23/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_OLDFEATURESMAP_H
#define FOREST_TREE_EVALUATOR_OLDFEATURESMAP_H

#include <vector>
#include <algorithm>
#include "Tree.h"
#include "Epitome.h"
#include "ResultMask.h"

class OldFeatureEvaluator {
	private:
		std::vector<double> splittingThreshold;
		std::vector<Epitome> masks;
		std::vector<int> treeIndexes;
		int featureIndex;
	public:
		OldFeatureEvaluator(
				const std::shared_ptr<Forest> &forest,
				const std::vector<std::vector<std::shared_ptr<InternalNode>>> &nodesByFeature,
				int featureIndex
		) : featureIndex(featureIndex) {
			auto nodes = nodesByFeature[featureIndex];
			std::sort(nodes.begin(), nodes.end(), [](auto &a, auto &b) -> bool {
				return a->splittingThreshold < b->splittingThreshold;
			});


			for (auto &n : nodes) {
				this->splittingThreshold.push_back(n->splittingThreshold);
				this->treeIndexes.push_back(n->getTreeIndex());

				this->masks.emplace_back(forest->trees[n->getTreeIndex()].countLeafsUntil(n), n->leftNode->numberOfLeafs());
			}
		}

		void evaluate(ResultMask &result, const std::vector<double> &element) const {
			double value = element[this->featureIndex];
			for (unsigned long i = 0; i < this->splittingThreshold.size(); i++) {
				if (value > this->splittingThreshold[i]) {
					result.applyMask(this->masks[i], this->treeIndexes[i]);
				} else {
					return;
				}
			}
		}
};

class OldFeaturesMap {
	private:
		std::shared_ptr<Forest> forest;
		std::vector<OldFeatureEvaluator> evaluators;
	public:

		OldFeaturesMap(std::shared_ptr<Forest> forest, int featuresCount) : forest(std::move(forest)) {
			auto nodesByFeature = this->forest->getNodesByFeature(featuresCount);
			for (int i = 0; i < featuresCount; i++) {
				evaluators.emplace_back(this->forest, nodesByFeature, i);
			}
		}

		[[nodiscard]] double score(const std::vector<double> &document) const {
			ResultMask result(this->forest);
#pragma omp parallel for if(PARALLEL_MASK) default(none) shared(result) shared(document)
			for (unsigned long i = 0; i < document.size(); i++) {
				this->evaluators[i].evaluate(result, document);
			}
			return result.computeScore();
		}
};


#endif //FOREST_TREE_EVALUATOR_OLDFEATURESMAP_H
