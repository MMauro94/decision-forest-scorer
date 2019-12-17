//
// Created by molin on 28/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_EPITOME_H
#define FOREST_TREE_EVALUATOR_EPITOME_H

#include <immintrin.h>
#include <stdexcept>
#include <iostream>
#include <bitset>
#include <vector>
#include <atomic>
#include <deque>
#include "config.h"

class SIMDEpitome {
	private:
		__m512i firstBlock;
		uint8_t firstBlockPosition;
		__m512i lastBlock;
		uint8_t lastBlockPosition;

		static constexpr inline auto Bits = 64;

	public:

		SIMDEpitome(const SIMDEpitome &other) = default;

		SIMDEpitome(unsigned int leftOnes, unsigned int middleZeroes) {
			if (middleZeroes == 0) {
				throw std::invalid_argument("middleZeroes == 0");
			}
			uint64_t one = 1u;

			uint64_t fb = 0;
			for (unsigned int i = 0; i < leftOnes % Bits; i++) {
				fb |= one << (Bits - i - one);
			}
			this->firstBlockPosition = leftOnes / Bits;

			uint64_t lb = 0;
			if ((middleZeroes + leftOnes) % Bits > 0) {
				for (unsigned int i = (middleZeroes + leftOnes) % Bits; i < Bits; i++) {
					lb |= one << (Bits - i - one);
				}
			}
			this->lastBlockPosition = (leftOnes + middleZeroes - one) / Bits;

			if (this->firstBlockPosition == this->lastBlockPosition) {
				fb |= lb;
				lb = fb;
			}

			this->firstBlock = _mm512_set1_epi64(fb);
			this->lastBlock = _mm512_set1_epi64(lb);
		}

		void performAnd(
				std::vector<__m512i> &results,
				unsigned int treeIndex,
				unsigned int masksPerTree,
				__mmask8 mask
		) const {
			unsigned int start = treeIndex * masksPerTree;


			results[start + this->firstBlockPosition] = _mm512_mask_and_epi64(
					results[start + this->firstBlockPosition],
					mask,
					this->firstBlock,
					results[start + this->firstBlockPosition]
			);

			if (this->firstBlockPosition != this->lastBlockPosition) {
				unsigned int end = start + this->lastBlockPosition;
				for (unsigned int i = start + this->firstBlockPosition + 1u; i < end; i++) {
					//TODO trovare un modo meglio
					results[i] = _mm512_mask_and_epi64(results[i], mask, SIMD_512_ZERO, results[i]);
				}
				results[end] = _mm512_mask_and_epi64(
						results[end],
						mask,
						this->lastBlock,
						results[end]
				);
			}

		}
};


#endif //FOREST_TREE_EVALUATOR_EPITOME_H
