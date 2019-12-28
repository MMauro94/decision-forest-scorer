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
#include "SIMDEpitome.h"

template<typename SIMDInfo>
class SIMDResultMask {
	private:
		typedef typename SIMDInfo::type simd_type;
		typedef typename SIMDInfo::base_type simd_base_type;

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

		void applyMask(const SIMDEpitome<SIMDInfo> &epitome, const unsigned int treeIndex, __mmask8 mask) {
			epitome.performAnd(this->results, treeIndex, this->masksPerTree, mask);
		}

		template<typename Scorer>
		[[nodiscard]] std::vector<double> computeScore(const Config<Scorer> &config) const {
			std::vector<double> scores(8, 0.0);
#pragma omp parallel for num_threads(config.number_of_threads) if(config.parallel_score) default(none) shared(scores)
			for (unsigned long i = 0; i < this->_forest->trees.size(); i++) {
				alignas(SIMDInfo::bits) simd_base_type leafIndexes[8];
				SIMDInfo::store(leafIndexes, this->firstOne(i));
				auto &tree = this->_forest->trees[i];
				for (unsigned int j = 0; j < 8; j++) {
					double s = tree.scoreByLeafIndex(leafIndexes[j]);
#pragma omp atomic update
					scores[j] += s;
				}
			}
			return scores;
		}

	private:

		[[nodiscard]] simd_type firstOne(unsigned long tree_index) const {
			__mmask8 found_results = 0xFF;
			simd_type result_indexes = SIMDInfo::set1(0);
			simd_type block_result = SIMDInfo::set1(0);
			simd_type group_size = SIMDInfo::set1(SIMDInfo::bits);

			for (unsigned long i = this->masksPerTree * tree_index; found_results > 0; i++) {
				block_result = SIMDInfo::mask_mov(block_result, found_results, this->results[i]);
				found_results = SIMDInfo::mask_cmp_mask(found_results, this->results[i], SIMDInfo::set1(0), _MM_CMPINT_EQ);
				result_indexes = SIMDInfo::mask_add(result_indexes, found_results, result_indexes, group_size);
			}

			block_result = SIMDInfo::lzcnt(block_result);
			return SIMDInfo::add(block_result, result_indexes);
		}

};

#endif //FOREST_TREE_EVALUATOR_SIMD_RESULTMASK_H
