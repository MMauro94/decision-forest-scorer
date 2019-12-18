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
#include "config.h"
#include "Tree.h"
#include "SIMDEpitome.h"

class SIMDResultMask {
	private:
		std::shared_ptr<Forest> _forest;
		unsigned int masksPerTree;
		std::vector<__m512i> results;

	public:

		explicit SIMDResultMask(std::shared_ptr<Forest> forest) :
				_forest(std::move(forest)),
				masksPerTree(this->_forest->maximumNumberOfLeafs() / 64 + 1) {
			__m512i ones = _mm512_set1_epi64(-1);
			for (unsigned long i = 0; i < this->_forest->trees.size() * this->masksPerTree; i++) {
				this->results.push_back(ones);
			}
		}

		void applyMask(const SIMDEpitome &epitome, const unsigned int treeIndex, __mmask8 mask) {
			epitome.performAnd(this->results, treeIndex, this->masksPerTree, mask);
		}

		[[nodiscard]] std::vector<double> computeScore() const {
			std::vector<double> scores(8, 0.0);
#pragma omp parallel for if(PARALLEL_SCORE) default(none) shared(scores)
			for (unsigned long i = 0; i < this->_forest->trees.size(); i++) {
				alignas(64) int64_t leafIndexes[8];
				_mm512_store_epi64(leafIndexes, this->firstOne(i));
				auto &tree = this->_forest->trees[i];
				for (unsigned int j = 0; j < 8; j++) {
					double s = tree.scoreByLeafIndex(leafIndexes[j]);
#pragma omp atomic
					scores[j] += s;
				}
			}
			return scores;
		}


		static void printAVX512i_binary(__m512i avx_integer) {
			alignas(64) int64_t local[8];
			_mm512_store_epi64(local, avx_integer);
			for (int i = 0; i < 8; i++) {
				auto bs = std::bitset<64>(local[i]);
				std::cout << bs << "|";
			}
			std::cout << std::endl;
		}

		static void printAVX512i_decimal(__m512i avx_integer) {
			alignas(64) int64_t local[8];
			_mm512_store_epi64(local, avx_integer);
			for (int i = 0; i < 8; i++) {
				std::cout << local[i] << ", ";
			}
			std::cout << std::endl;
		}

	private:

		[[nodiscard]] __m512i firstOne(unsigned long treeIndex) const {
			__mmask8 foundResults = 0xFF;
			__m512i resultIndexes = _mm512_set1_epi64(0);
			__m512i blockResult = _mm512_set1_epi64(0);

			__m512i sixtyfour = _mm512_set1_epi64(64);
			unsigned long mult = this->masksPerTree * treeIndex;

			unsigned long i = mult;
			while (foundResults > 0) {
				blockResult = _mm512_mask_mov_epi64(blockResult, foundResults, this->results[i]);
				foundResults = _mm512_mask_cmp_epi64_mask(foundResults, this->results[i], _mm512_set1_epi64(0),
														  _MM_CMPINT_EQ);
				resultIndexes = _mm512_mask_add_epi64(resultIndexes, foundResults, resultIndexes, sixtyfour);
				i++;
			}

			blockResult = _mm512_lzcnt_epi64(blockResult);
			return _mm512_add_epi64(blockResult, resultIndexes);;
		}

};

#endif //FOREST_TREE_EVALUATOR_SIMD_RESULTMASK_H
