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
#include "Epitome.h"

template<typename SIMDInfo>
class SIMDEpitome : public Epitome<typename SIMDInfo::base_type> {

		typedef typename SIMDInfo::type simd_type;
		typedef typename SIMDInfo::base_type simd_base_type;

	public:
		SIMDEpitome(const SIMDEpitome<SIMDInfo> &other) = default;

		SIMDEpitome(unsigned int leftOnes, unsigned int middleZeroes) : Epitome<simd_base_type>(leftOnes, middleZeroes) {}


		void performAnd(
				std::vector<simd_type> &results,
				unsigned int treeIndex,
				unsigned int masksPerTree,
				__mmask8 mask
		) const {
			unsigned int start = treeIndex * masksPerTree;

			results[start + this->firstBlockPosition] = SIMDInfo::mask_and(
					results[start + this->firstBlockPosition],
					mask,
					SIMDInfo::set1(this->firstBlock),
					results[start + this->firstBlockPosition]
			);

			if (this->firstBlockPosition != this->lastBlockPosition) {
				unsigned int end = start + this->lastBlockPosition;
				for (unsigned int i = start + this->firstBlockPosition + 1u; i < end; i++) {
					results[i] = SIMDInfo::mask_set1(results[i], mask, 0);
				}

				results[end] = SIMDInfo::mask_and(
						results[end],
						mask,
						SIMDInfo::set1(this->lastBlock),
						results[end]
				);
			}

		}
};


#endif //FOREST_TREE_EVALUATOR_SIMD_EPITOME_H
