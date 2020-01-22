#ifndef FOREST_TREE_EVALUATOR_EQNODESRAPIDSCORER_H
#define FOREST_TREE_EVALUATOR_EQNODESRAPIDSCORER_H

#include <memory>
#include "../Tree.h"
#include "../EqNode.h"
#include "../ResultMask.h"
#include "../DocGroup.h"
#include "../Config.h"

/**
 * The basic implementation of RapidScorer. The data is saved inside EqNode.
 */
template <typename Block>
class EqNodesRapidScorer {

		Config<EqNodesRapidScorer<Block>> config;
		std::shared_ptr<Forest> forest;
		std::vector<EqNode<Block>> eqNodes;
		std::vector<unsigned int> offsets;

		void addNodes(const std::shared_ptr<InternalNode> &node) {
			eqNodes.emplace_back(node, Epitome<Block>(this->forest->trees[node->getTreeIndex()].countLeavesUntil(node),
													  node->leftNode->numberOfLeaves()));

			auto leftAsInternalNode = std::dynamic_pointer_cast<InternalNode>(node->leftNode);
			auto rightAsInternalNode = std::dynamic_pointer_cast<InternalNode>(node->rightNode);
			if (leftAsInternalNode != nullptr) {
				addNodes(leftAsInternalNode);
			}
			if (rightAsInternalNode != nullptr) {
				addNodes(rightAsInternalNode);
			}
		}


	public:
		typedef SingleDocument DocGroup;

		explicit EqNodesRapidScorer(const Config<EqNodesRapidScorer<Block>> &config, std::shared_ptr<Forest> forest) : config(config), forest(std::move(forest)) {
			for (auto &tree : this->forest->trees) {
				addNodes(tree.root);
			}
			std::sort(this->eqNodes.begin(), this->eqNodes.end());
			int i = 0;
			for (auto &node : this->eqNodes) {
				while (this->offsets.size() <= node.featureIndex) {
					this->offsets.emplace_back(i);
				}
				i++;
			}
		}

		[[nodiscard]] std::vector<double> score(const DocGroup &document) const {
			ResultMask<Block> result(this->forest);

			unsigned long max = this->offsets.size();
#pragma omp parallel for num_threads(this->config.number_of_threads) if(this->config.parallel_features) default(none) shared(result) shared(document) shared(max)
			for (unsigned long featureIndex = 0; featureIndex < max; featureIndex++) {
				double value = document.features[featureIndex];
				for (unsigned long j = this->offsets[featureIndex];
					 this->eqNodes[j].featureIndex == featureIndex; j++) {

					const EqNode<Block> &node = this->eqNodes[j];
					if (value > node.featureThreshold) {
						result.applyMask(node.epitome, node.treeIndex);
					} else {
						break;
					}
				}
			}

			return {result.computeScore(this->config)};
		}
};

#endif //FOREST_TREE_EVALUATOR_EQNODESRAPIDSCORER_H
