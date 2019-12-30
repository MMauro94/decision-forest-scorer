#ifndef FOREST_TREE_EVALUATOR_SIMDRAPIDSCORER_H
#define FOREST_TREE_EVALUATOR_SIMDRAPIDSCORER_H

#include <memory>
#include <algorithm>
#include <immintrin.h>
#include "../Tree.h"
#include "../SIMDDoubleGroup.h"
#include "../DocGroup.h"
#include "../Epitome.h"
#include "../Config.h"
#include "../SIMDResultMask.h"

template <typename SIMDInfo>
class SIMDRapidScorer {
		typedef typename SIMDInfo::base_type simd_base_type;

		Config<SIMDRapidScorer> config;
		std::shared_ptr<Forest> forest;
		SIMDDoubleGroup featureThresholds;//TODO: salvare normalmente e fare set1 al bisogno?
		std::vector<unsigned int> treeIndexes;
		std::vector<Epitome<simd_base_type>> epitomes;
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
			if (node1->splittingFeatureIndex < node2->splittingFeatureIndex) { return true; }
			else if (node1->splittingFeatureIndex > node2->splittingFeatureIndex) { return false; }
			else { return node1->splittingThreshold < node2->splittingThreshold; }
		}

	public:
		typedef SIMDDocumentGroup DocGroup;

		explicit SIMDRapidScorer(const Config<SIMDRapidScorer> &config, std::shared_ptr<Forest> forest) : config(config), forest(std::move(forest)) {
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

		[[nodiscard]] std::vector<double> score(const DocGroup &documents) const {
			SIMDResultMask<SIMDInfo> result(this->forest);

			unsigned long max = this->offsets.size();
#pragma omp parallel for num_threads(this->config.number_of_threads) if(this->config.parallel_mask) default(none) shared(result) shared(documents) shared(max)
			for (unsigned long featureIndex = 0; featureIndex < max; featureIndex++) {
				__m512d value = documents.get(featureIndex);
				unsigned int start = this->offsets[featureIndex];
				unsigned int end;
				if (featureIndex + 1 < this->offsets.size()) {
					end = this->offsets[featureIndex + 1];
				} else {
					end = this->featureThresholds.size();
				}

				__mmask8 isLE = 0xFF;
				for (unsigned int i = start; i < end && isLE > 0; i++) {
					// extract mask of comparison 1 if the comparison is FALSE
					isLE = _mm512_mask_cmp_pd_mask(isLE, value, this->featureThresholds.get(i), _CMP_GT_OQ);
					result.applyMask(this->epitomes[i], this->treeIndexes[i], isLE);
				}
			}

			return result.computeScore(this->config);
		}
};

#endif //FOREST_TREE_EVALUATOR_SIMDRAPIDSCORER_H
