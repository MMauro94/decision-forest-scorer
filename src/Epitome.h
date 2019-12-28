//
// Created by molin on 28/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_EPITOME_H
#define FOREST_TREE_EVALUATOR_EPITOME_H

#include <stdexcept>
#include <iostream>
#include <bitset>
#include <vector>
#include <atomic>
#include <deque>
#include <immintrin.h>
#include "Config.h"

template<typename Block>
class Epitome {
	private:
		Block firstBlock;
		uint8_t firstBlockPosition;
		Block lastBlock;
		uint8_t lastBlockPosition;

		static constexpr inline auto Bits = sizeof(Block) * 8;

	public:

		Epitome(const Epitome &other) = default;

		Epitome(unsigned int leftOnes, unsigned int middleZeroes) {
			if (middleZeroes == 0) {
				throw std::invalid_argument("middleZeroes == 0");
			}
			Block one = 1u;

			this->firstBlock = 0;
			for (unsigned int i = 0; i < leftOnes % Bits; i++) {
				this->firstBlock |= one << (Bits - i - one);
			}
			this->firstBlockPosition = leftOnes / Bits;

			this->lastBlock = 0;
			if ((middleZeroes + leftOnes) % Bits > 0) {
				for (unsigned int i = (middleZeroes + leftOnes) % Bits; i < Bits; i++) {
					this->lastBlock |= one << (Bits - i - one);
				}
			}
			this->lastBlockPosition = (leftOnes + middleZeroes - one) / Bits;

			if (this->firstBlockPosition == this->lastBlockPosition) {
				this->firstBlock |= this->lastBlock;
				this->lastBlock = this->firstBlock;
			}
		}

		void performAnd(std::vector<Block> &results, unsigned int treeIndex, unsigned int masksPerTree) const {
			unsigned int start = treeIndex * masksPerTree;
#pragma omp atomic update
			results[start + this->firstBlockPosition] &= this->firstBlock;
			if (this->firstBlockPosition != this->lastBlockPosition) {
				unsigned int end = start + this->lastBlockPosition;
				for (unsigned int i = start + this->firstBlockPosition + 1u; i < end; i++) {
#pragma omp atomic write
					results[i] = 0;
				}
#pragma omp atomic update
				results[end] &= this->lastBlock;
			}
		}

		typename ::std::enable_if<Bits == 64, void> performAnd(
				std::vector<__m512i> &results,
				unsigned int treeIndex,
				unsigned int masksPerTree,
				__mmask8 mask
		) const {
			unsigned int start = treeIndex * masksPerTree;

			results[start + this->firstBlockPosition] = _mm512_mask_and_epi64(
					results[start + this->firstBlockPosition],
					mask,
					_mm512_set1_epi64((long long int) this->firstBlock),
					results[start + this->firstBlockPosition]
			);

			if (this->firstBlockPosition != this->lastBlockPosition) {
				unsigned int end = start + this->lastBlockPosition;
				for (unsigned int i = start + this->firstBlockPosition + 1u; i < end; i++) {
					results[i] = _mm512_mask_set1_epi64(results[i], mask, 0);
				}

				results[end] = _mm512_mask_and_epi64(
						results[end],
						mask,
						_mm512_set1_epi64((long long int) this->lastBlock),
						results[end]
				);
			}

		}

		friend std::ostream &operator<<(std::ostream &os, const Epitome &epitome) {
			os << epitome.toString(true);
			return os;
		}

		[[nodiscard]] std::string toString(bool separateBytes) const {
			std::string ret;

			for (int i = 0; i < this->firstBlockPosition; i++) {
				for (int j = 0; j < Bits; j++) {
					ret += "1";
				}
				if (separateBytes) {
					ret += " ";
				}
			}
			if (this->lastBlockPosition != this->firstBlockPosition) {
				std::bitset<Bits> fb(this->firstBlock);
				ret += fb.to_string();
				if (separateBytes) {
					ret += " ";
				}
			}

			for (int i = this->firstBlockPosition + 1; i < this->lastBlockPosition; i++) {
				for (int j = 0; j < Bits; j++) {
					ret += "0";
				}
				if (separateBytes) {
					ret += " ";
				}
			}
			std::bitset<Bits> lb(this->lastBlock);
			ret += lb.to_string();
			return ret;
		}
};


#endif //FOREST_TREE_EVALUATOR_EPITOME_H
