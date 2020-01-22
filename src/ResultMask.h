//
// Created by molin on 28/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_RESULTMASK_H
#define FOREST_TREE_EVALUATOR_RESULTMASK_H


#include <vector>
#include <mutex>
#include <deque>
#include <atomic>
#include <strings.h>
#include <immintrin.h>
#include "Config.h"
#include "Tree.h"
#include "Epitome.h"
#include <immintrin.h>

/**
 * This class allows to update the scores of the documents with an epitome.
 */
template<typename Block>
class ResultMask {
	private:
		std::shared_ptr<Forest> _forest;
		unsigned int masksPerTree;
		std::vector<Block> results;

		static constexpr int MaskSize = (sizeof(Block) * 8);

	public:

		explicit ResultMask(std::shared_ptr<Forest> forest) :
				_forest(std::move(forest)),
				masksPerTree(this->_forest->maximumNumberOfLeaves() / MaskSize + 1),
				results(this->_forest->trees.size() * this->masksPerTree, -1) {}

		/**
		 * Changes the result masks of the document according to the given epitome.
		 */
		void applyMask(const Epitome<Block> &epitome, const unsigned int treeIndex) {
			applyMask(epitome.firstBlock, epitome.firstBlockPosition, epitome.lastBlock, epitome.lastBlockPosition, treeIndex);
		}

		/**
		 * Changes the result masks of the document according to the given epitome data
		 */
		void applyMask(Block firstBlock, uint8_t firstBlockPosition, Block lastBlock, uint8_t lastBlockPosition, const unsigned int treeIndex) {
			unsigned int start = treeIndex * masksPerTree;
#pragma omp atomic update//TODO: fa atomic anche quando è in un parallelismo per cui atomic non serve?
			this->results[start + firstBlockPosition] &= firstBlock;
			if (firstBlockPosition != lastBlockPosition) {
				unsigned int end = start + lastBlockPosition;
				for (unsigned int i = start + firstBlockPosition + 1u; i < end; i++) {
#pragma omp atomic write//TODO: fa atomic anche quando è in un parallelismo per cui atomic non serve?
					this->results[i] = 0;
				}
#pragma omp atomic update//TODO: fa atomic anche quando è in un parallelismo per cui atomic non serve?
				this->results[end] &= lastBlock;
			}
		}

		/**
		 * Compute the scored of the documents according to the internal result mask.
		 */
		template<typename Scorer>
		[[nodiscard]] double computeScore(const Config<Scorer> &config) const {
			double score = 0.0;
#pragma omp parallel for num_threads(config.number_of_threads) if(config.parallel_score) default(none) reduction(+:score)
			for (unsigned long i = 0; i < this->_forest->trees.size(); i++) {
				auto leafIndex = this->firstOne(i);
				auto &tree = this->_forest->trees[i];
				score += tree.scoreByLeafIndex(leafIndex);//TODO: capire se c'è un modo meglio di ottenere questo score
			}
			return score;
		}

	private:

		/**
		 * Calculates the first bit set to 1 in the result mask
		 */
		[[nodiscard]] unsigned long firstOne(unsigned long treeIndex) const {
			unsigned long baseIndex = this->masksPerTree * treeIndex;

			unsigned long i = 0;
			while (this->results[baseIndex + i] == 0) {
				i++;
			}
			auto firstNonZeroBlock = this->results[baseIndex + i];

			if (MaskSize == 64) {
				return i * MaskSize + _lzcnt_u64(firstNonZeroBlock);
			} else if (MaskSize == 32) {
				return i * MaskSize + _lzcnt_u32(firstNonZeroBlock);
			} else if (MaskSize < 32) {
				return i * MaskSize + _lzcnt_u32((uint32_t) firstNonZeroBlock) + MaskSize - 32;
			} else if (MaskSize < 64) {
				return i * MaskSize + _lzcnt_u64((uint64_t) firstNonZeroBlock) + MaskSize - 64;
			} else {
				unsigned int j = 0;
				for (; j < MaskSize; j++) {
					if ((firstNonZeroBlock >> (MaskSize - 1 - j)) % 2 == 1) { break; }
				}

				return i * MaskSize + j;

			}
		}

};

#endif //FOREST_TREE_EVALUATOR_RESULTMASK_H
