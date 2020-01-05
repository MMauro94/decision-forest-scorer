//
// Created by molin on 28/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_SIMD_RESULTMASK_H
#define FOREST_TREE_EVALUATOR_SIMD_RESULTMASK_H


#include <immintrin.h>
#include <vector>
#include <mutex>
#include <deque>
#include <atomic>
#include <strings.h>
#include "Config.h"
#include "Tree.h"
#include "Epitome.h"

template<typename SIMDInfo>
class SIMDResultMask {
	private:
		typedef typename SIMDInfo::type simd_type;
		typedef typename SIMDInfo::base_type simd_base_type;
		typedef typename SIMDInfo::mask_type simd_mask_type;

		std::shared_ptr<Forest> _forest;
		unsigned int masksPerTree;
		std::vector<simd_type> results;


	public:

		explicit SIMDResultMask(std::shared_ptr<Forest> forest) :
				_forest(std::move(forest)),
				masksPerTree(this->_forest->maximumNumberOfLeafs() / SIMDInfo::bits + 1) {
			simd_type ones = SIMDInfo::set1(-1);
			for (unsigned long i = 0; i < this->_forest->trees.size() * this->masksPerTree; i++) {
				this->results.push_back(ones);
			}
		}

		void applyMask(const Epitome<simd_base_type> &epitome, const unsigned int treeIndex, simd_mask_type mask) {
			this->applyMask(epitome.firstBlock, epitome.firstBlockPosition, epitome.lastBlock, epitome.lastBlockPosition, treeIndex, mask);
		}

		void applyMask(simd_base_type firstBlock, u_int8_t firstBlockPosition, simd_base_type lastBlock, u_int8_t lastBlockPosition, const unsigned int treeIndex, simd_mask_type mask) {
			unsigned int start = treeIndex * this->masksPerTree;

			this->results[start + firstBlockPosition] = SIMDInfo::mask_and(
					this->results[start + firstBlockPosition],
					mask,
					SIMDInfo::set1(firstBlock),
					this->results[start + firstBlockPosition]
			);

			if (firstBlockPosition != lastBlockPosition) {
				unsigned int end = start + lastBlockPosition;
				for (unsigned int i = start + firstBlockPosition + 1u; i < end; i++) {
					this->results[i] = SIMDInfo::mask_set1(this->results[i], mask, 0);
				}

				this->results[end] = SIMDInfo::mask_and(
						this->results[end],
						mask,
						SIMDInfo::set1(lastBlock),
						this->results[end]
				);
			}
		}

		template<typename Scorer>
		[[nodiscard]] std::vector<double> computeScore(const Config<Scorer> &config) const {
			const auto simdGroups = SIMDInfo::groups;
			std::vector<double> scores(simdGroups, 0.0);
#pragma omp parallel for num_threads(config.number_of_threads) if(config.parallel_score) default(none) shared(scores)
			for (unsigned long i = 0; i < this->_forest->trees.size(); i++) {
				if (sizeof(simd_base_type) > 1) {
					alignas(SIMDInfo::bits) simd_base_type leafIndexes[simdGroups];
					firstOne(i, leafIndexes);
					this->updateScores(scores, i, leafIndexes);
				} else {
					//I cannot store the leaf index inside simd_base_type (max number of leaves > 255)
					unsigned int leafIndexes[simdGroups];
					firstOneArray(i, leafIndexes);
					this->updateScores(scores, i, leafIndexes);
				}
			}
			return scores;
		}

	private:

		template<typename T>
		void updateScores(std::vector<double> &scores, unsigned long treeIndex, T leafIndexes[]) const {
			auto &tree = this->_forest->trees[treeIndex];
			for (unsigned int j = 0; j < SIMDInfo::groups; j++) {
				double s = tree.scoreByLeafIndex(leafIndexes[j]);
#pragma omp atomic update //TODO: fa atomic anche quando è in un parallelismo per cui atomic non serve?
				scores[j] += s;
			}
		}

		void firstOne(unsigned long tree_index, simd_base_type toFill[]) const {
			simd_mask_type found_results = -1;
			simd_type zero = SIMDInfo::setZero();
			simd_type result_indexes = zero;
			simd_type block_result = zero;
			simd_type group_size = SIMDInfo::set1(SIMDInfo::bits);

			for (unsigned long i = this->masksPerTree * tree_index; found_results != 0; i++) {
				block_result = SIMDInfo::mask_mov(block_result, found_results, this->results[i]);
				found_results = SIMDInfo::mask_eq(found_results, this->results[i], zero);
				result_indexes = SIMDInfo::mask_add(result_indexes, found_results, result_indexes, group_size);
			}

			simd_type lzcnt = SIMDInfo::lzcnt(block_result);
			simd_type result = SIMDInfo::add(lzcnt, result_indexes);

			SIMDInfo::store(toFill, result);
		}

		void firstOneArray(unsigned long tree_index, unsigned int toFill[]) const {
			simd_mask_type found_results = -1;
			simd_type zero = SIMDInfo::setZero();
			simd_type one = SIMDInfo::set1(1);
			simd_type result_indexes = zero;
			simd_type block_result = zero;

			for (unsigned long i = this->masksPerTree * tree_index; found_results != 0; i++) {
				block_result = SIMDInfo::mask_mov(block_result, found_results, this->results[i]);
				found_results = SIMDInfo::mask_eq(found_results, this->results[i], zero);
				result_indexes = SIMDInfo::mask_add(result_indexes, found_results, result_indexes, one);
			}
			simd_type lzcnt = SIMDInfo::lzcnt(block_result);

			alignas(SIMDInfo::bits) simd_base_type result_indexes_array[SIMDInfo::groups];
			alignas(SIMDInfo::bits) simd_base_type lzcnt_array[SIMDInfo::groups];
			SIMDInfo::store(result_indexes_array, result_indexes);
			SIMDInfo::store(lzcnt_array, lzcnt);

			for (int i = 0; i < SIMDInfo::groups; i++) {
				toFill[i] = result_indexes_array[i] * SIMDInfo::bits + lzcnt_array[i];
			}
		}

};

#endif //FOREST_TREE_EVALUATOR_SIMD_RESULTMASK_H
