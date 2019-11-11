//
// Created by molin on 11/11/2019.
//

#ifndef FOREST_TREE_EVALUATOR_RAPIDSCORERE_H
#define FOREST_TREE_EVALUATOR_RAPIDSCORERE_H

#include <memory>
#include <algorithm>
#include "Tree.h"
#include "EqNode.h"
#include "ResultMask.h"

class RapidScorer {
		std::shared_ptr<Forest> forest;
		std::vector<EqNode> nodes;
		std::vector<unsigned int> offsets;

		void addNodes(const std::shared_ptr<InternalNode> &node) {
			nodes.emplace_back(node, Epitome(this->forest->trees[node->getTreeIndex()].countLeafsUntil(node),
											 node->leftNode->numberOfLeafs()));

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
		RapidScorer(std::shared_ptr<Forest> forest) : forest(std::move(forest)) {
			for (auto &tree : this->forest->trees) {
				addNodes(tree.root);
			}
			std::sort(this->nodes.begin(), this->nodes.end());
			int i = 0;
			for (auto &node : this->nodes) {
				while (this->offsets.size() <= node.featureIndex) {
					offsets.emplace_back(i);
				}
				i++;
			}
		}

		[[nodiscard]] double score(const std::vector<double> &document) const {
			ResultMask result(this->forest);

#pragma omp parallel for if(PARALLEL_MASK) default(none) shared(result) shared(document)
			for (unsigned long featureIndex = 0; featureIndex < document.size(); featureIndex++) {
				double value = document[featureIndex];
				for (unsigned long j = this->offsets[featureIndex]; this->nodes[j].featureIndex == featureIndex; j++) {
					const EqNode &node = this->nodes[j];
					if (value > node.featureThreshold) {
						result.applyMask(node.epitome, node.treeIndex);
					} else {
						break;
					}
				}
			}

			return result.computeScore();
		}

};

#endif //FOREST_TREE_EVALUATOR_RAPIDSCORERE_H
