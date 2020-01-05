#ifndef FOREST_TREE_EVALUATOR_MERGEDRAPIDSCORER_H
#define FOREST_TREE_EVALUATOR_MERGEDRAPIDSCORER_H

#include <memory>
#include <algorithm>
#include "../Tree.h"
#include "../ResultMask.h"
#include "../DocGroup.h"
#include "../Config.h"

template<typename Block>
class SingleFeatureMergedRapidScorer;

template<typename Block>
class MergedRapidScorer {

		Config<MergedRapidScorer<Block>> config;
		std::shared_ptr<Forest> forest;
		std::vector<SingleFeatureMergedRapidScorer<Block>> featureScorers;

	public:
		typedef SingleDocument DocGroup;

		explicit MergedRapidScorer(const Config<MergedRapidScorer<Block>> &config, std::shared_ptr<Forest> forest) : config(config), forest(std::move(forest)) {
			auto featuresCount = this->forest->maximumFeatureIndex();
			for (unsigned int i = 0; i <= featuresCount; i++) {
				this->featureScorers.emplace_back(this->forest, i);
			}
		}

		[[nodiscard]] std::vector<double> score(const DocGroup &document) const {
			ResultMask<Block> result(this->forest);

			unsigned long max = this->featureScorers.size();
#pragma omp parallel for num_threads(this->config.number_of_threads) if(this->config.parallel_mask) default(none) shared(result) shared(document) shared(max)
			for (unsigned long featureIndex = 0; featureIndex < max; featureIndex++) {
				featureScorers[featureIndex].score(document, result);
			}

			return {result.computeScore(this->config)};
		}
};

template<typename Block>
class SingleFeatureMergedRapidScorer {
	protected:
		unsigned int featureIndex;

		std::vector<double> featureThresholds;
		std::vector<unsigned int> treeIndexes;
		std::vector<unsigned int> featureThresholdToOffset;

		std::vector<Block> firstBlocks;
		std::vector<uint8_t> firstBlockPositions;
		std::vector<Block> lastBlocks;
		std::vector<uint8_t> lastBlockPositions;

		std::vector<bool> lastBlockIsSameAsFirstBlock;


	private:
		void addNodes(std::vector<std::shared_ptr<InternalNode>> &ret, const std::shared_ptr<InternalNode> &node) const {
			if (node->splittingFeatureIndex == this->featureIndex) {
				ret.push_back(node);
			}

			auto leftAsInternalNode = std::dynamic_pointer_cast<InternalNode>(node->leftNode);
			auto rightAsInternalNode = std::dynamic_pointer_cast<InternalNode>(node->rightNode);
			if (leftAsInternalNode != nullptr) {
				addNodes(ret, leftAsInternalNode);
			}
			if (rightAsInternalNode != nullptr) {
				addNodes(ret, rightAsInternalNode);
			}
		}

		[[nodiscard]] static bool nodeComparator(const std::shared_ptr<InternalNode> &node1, const std::shared_ptr<InternalNode> &node2) {
			return node1->splittingThreshold < node2->splittingThreshold;
		}

	public:
		typedef SingleDocument DocGroup;

		explicit SingleFeatureMergedRapidScorer(const std::shared_ptr<Forest> &forest, unsigned int featureIndex) : featureIndex(featureIndex) {
			std::vector<std::shared_ptr<InternalNode>> nodes;
			for (auto &tree : forest->trees) {
				addNodes(nodes, tree.root);
			}

			std::sort(nodes.begin(), nodes.end(), nodeComparator);

			for (unsigned int i = 0; i < nodes.size(); i++) {
				auto &node = nodes[i];
				if (i == 0 || nodes[i - 1]->splittingThreshold != node->splittingThreshold) {
					this->featureThresholds.emplace_back(node->splittingThreshold);
					this->featureThresholdToOffset.emplace_back(i);
				}
				this->treeIndexes.emplace_back(node->getTreeIndex());
				auto ep = Epitome<Block>(forest->trees[node->getTreeIndex()].countLeafsUntil(node), node->leftNode->numberOfLeafs());
				this->firstBlocks.emplace_back(ep.firstBlock);
				this->firstBlockPositions.emplace_back(ep.firstBlockPosition);
				if (ep.firstBlockPosition != ep.lastBlockPosition) {
					this->lastBlocks.emplace_back(ep.lastBlock);
					this->lastBlockPositions.emplace_back(ep.lastBlockPosition);
					this->lastBlockIsSameAsFirstBlock.emplace_back(false);
				} else {
					this->lastBlockIsSameAsFirstBlock.emplace_back(true);
				}
			}
			this->featureThresholdToOffset.emplace_back(nodes.size());
			/*std::cout << "Feature " << featureIndex << ": " << nodes.size() << " nodes, \t" <<
					  this->lastBlocks.size() << " last blocks, \t" <<
					  this->featureThresholds.size() << " thresholds, \t" <<
					  std::endl;*/
		}
		void score(const DocGroup &document, ResultMask<Block> &result) const {
			unsigned long thresholdIndex = std::lower_bound(this->featureThresholds.begin(), this->featureThresholds.end(), document.features[this->featureIndex]) - this->featureThresholds.begin();
			unsigned int epitomesToEpitome = this->featureThresholdToOffset[thresholdIndex];
			unsigned int lastBlockIndex = 0;

			for (unsigned long index = 0; index < epitomesToEpitome; index++) {
				if (this->lastBlockIsSameAsFirstBlock[index]) {
					result.applyMask(
							this->firstBlocks[index],
							this->firstBlockPositions[index],
							this->firstBlocks[index],
							this->firstBlockPositions[index],
							this->treeIndexes[index]
					);
				} else {
					result.applyMask(
							this->firstBlocks[index],
							this->firstBlockPositions[index],
							this->lastBlocks[lastBlockIndex],
							this->lastBlockPositions[lastBlockIndex],
							this->treeIndexes[index]
					);
					lastBlockIndex++;
				}
			}
		}
};

#endif //FOREST_TREE_EVALUATOR_MERGEDRAPIDSCORER_H
