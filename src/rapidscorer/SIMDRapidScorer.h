#ifndef FOREST_TREE_EVALUATOR_SIMDRAPIDSCORER_H
#define FOREST_TREE_EVALUATOR_SIMDRAPIDSCORER_H

#include <memory>
#include <algorithm>
#include <immintrin.h>
#include "../Tree.h"
#include "../SIMDDoubleGroup.h"
#include "../SIMDResultMask.h"

class SIMDRapidScorer {
		std::shared_ptr<Forest> forest;
		SIMDDoubleGroup featureThresholds;
		std::vector<unsigned int> treeIndexes;
		std::vector<SIMDEpitome> epitomes;
		std::vector<unsigned int> offsets;

		static void
		addNodes(std::vector<std::shared_ptr<InternalNode>> &ret, const std::shared_ptr<InternalNode> &node) {
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
		explicit SIMDRapidScorer(std::shared_ptr<Forest> forest) : forest(std::move(forest)) {
			std::vector<std::shared_ptr<InternalNode>> nodes;
			for (auto &tree : this->forest->trees) {
				addNodes(nodes, tree.root);
			}

			std::sort(nodes.begin(), nodes.end(), nodeComparator);

			int i = 0;
			for (auto &node : nodes) {
				this->featureThresholds.addEightTimes(node->splittingThreshold);

				this->treeIndexes.emplace_back(node->getTreeIndex());
				this->epitomes.emplace_back(this->forest->trees[node->getTreeIndex()].countLeafsUntil(node),
											node->leftNode->numberOfLeafs());

				while (this->offsets.size() <= node->splittingFeatureIndex) {
					this->offsets.emplace_back(i);
				}
				i++;
			}
		}

		[[nodiscard]] std::vector<double> score(const SIMDDoubleGroup &documents) const {
			SIMDResultMask result(this->forest);

			unsigned long max = this->offsets.size();
#pragma omp parallel for if(PARALLEL_MASK) default(none) shared(result) shared(documents) shared(max)
			for (unsigned long featureIndex = 0; featureIndex < max; featureIndex++) {
				__m512d value = documents.get(featureIndex);
				unsigned int start = this->offsets[featureIndex];

				__mmask8 isLE = 0xFF;
				unsigned int i = start;
				while (isLE > 0) {
					// extract mask of comparison 1 if the comparison is FALSE
					isLE = _mm512_mask_cmp_pd_mask(isLE, value, this->featureThresholds.get(i), _CMP_GT_OQ);
					result.applyMask(this->epitomes[i], this->treeIndexes[i], isLE);
					i++;
				}
			}

			return result.computeScore();
		}
};

#endif //FOREST_TREE_EVALUATOR_SIMDRAPIDSCORER_H
