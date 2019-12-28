#ifndef FOREST_TREE_EVALUATOR_LINEARIZEDRAPIDSCORER_H
#define FOREST_TREE_EVALUATOR_LINEARIZEDRAPIDSCORER_H

#include <memory>
#include <algorithm>
#include "../Tree.h"
#include "../ResultMask.h"
#include "../DocGroup.h"

class LinearizedRapidScorer {

		std::shared_ptr<Forest> forest;
		std::vector<double> featureThresholds;
		std::vector<unsigned int> treeIndexes;
		std::vector<Epitome<BLOCK>> epitomes;
		std::vector<unsigned int> offsets;

		static void addNodes(std::vector<std::shared_ptr<InternalNode>> &ret, const std::shared_ptr<InternalNode> &node) {
			ret.push_back(node);

			auto leftAsInternalNode = std::dynamic_pointer_cast<InternalNode>(node->leftNode);
			auto rightAsInternalNode = std::dynamic_pointer_cast<InternalNode>(node->rightNode);
			if (leftAsInternalNode != nullptr) {
				addNodes(ret, leftAsInternalNode);
			}
			if (rightAsInternalNode != nullptr) {
				addNodes(ret, rightAsInternalNode);
			}
		}

		[[nodiscard]] static bool
		nodeComparator(const std::shared_ptr<InternalNode> &node1, const std::shared_ptr<InternalNode> &node2) {
			if (node1->splittingFeatureIndex < node2->splittingFeatureIndex) return true;
			else if (node1->splittingFeatureIndex > node2->splittingFeatureIndex) return false;
			else return node1->splittingThreshold < node2->splittingThreshold;
		}

	public:
		typedef SingleDocument DocGroup;

		explicit LinearizedRapidScorer(std::shared_ptr<Forest> forest) : forest(std::move(forest)) {
			std::vector<std::shared_ptr<InternalNode>> nodes;
			for (auto &tree : this->forest->trees) {
				addNodes(nodes, tree.root);
			}

			std::sort(nodes.begin(), nodes.end(), nodeComparator);

			int i = 0;
			for (auto &node : nodes) {
				this->featureThresholds.emplace_back(node->splittingThreshold);
				this->treeIndexes.emplace_back(node->getTreeIndex());
				this->epitomes.emplace_back(this->forest->trees[node->getTreeIndex()].countLeafsUntil(node),
											node->leftNode->numberOfLeafs());

				while (this->offsets.size() <= node->splittingFeatureIndex) {
					this->offsets.emplace_back(i);
				}
				i++;
			}
		}

		[[nodiscard]] std::vector<double> score(const DocGroup &document) const {
			ResultMask result(this->forest);

			unsigned long max = this->offsets.size();
#pragma omp parallel for num_threads(NUMBER_OF_THREADS) if(PARALLEL_MASK) default(none) shared(result) shared(document) shared(max)
			for (unsigned long featureIndex = 0; featureIndex < max; featureIndex++) {
				double value = document.features[featureIndex];
				unsigned int start = this->offsets[featureIndex];
				unsigned int end;
				if (featureIndex + 1 < this->offsets.size()) {
					end = this->offsets[featureIndex + 1];
				} else {
					end = this->featureThresholds.size();
				}


				unsigned long epitomesToEpitome = std::lower_bound(
						this->featureThresholds.begin() + start,
						this->featureThresholds.begin() + end,
						value
				) - this->featureThresholds.begin();

				for (unsigned long j = start; j < epitomesToEpitome; j++) {
					result.applyMask(this->epitomes[j], this->treeIndexes[j]);
				}
			}

			return {result.computeScore()};
		}
};

#endif //FOREST_TREE_EVALUATOR_LINEARIZEDRAPIDSCORER_H
