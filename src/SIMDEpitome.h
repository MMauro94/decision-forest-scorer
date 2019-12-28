//
// Created by molin on 28/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_SIMD_EPITOME_H
#define FOREST_TREE_EVALUATOR_SIMD_EPITOME_H

#include <immintrin.h>
#include <stdexcept>
#include <iostream>
#include <bitset>
#include <vector>
#include <atomic>
#include <deque>
#include "Config.h"

class SIMDEpitome {
	private:
		uint64_t firstBlock;
		uint8_t firstBlockPosition;
		uint64_t lastBlock;
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

			this->firstBlock = fb;
			this->lastBlock = lb;
		}


};


#endif //FOREST_TREE_EVALUATOR_SIMD_EPITOME_H
