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
#include "config.h"

class SIMDEpitome {
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

		void performAnd(std::vector<MaskType> &masks, unsigned int treeIndex, unsigned int masksPerTree) const {
			unsigned int start = treeIndex * masksPerTree;

			masks[start + this->firstBlockPosition] &= this->firstBlock;
			if (this->firstBlockPosition != this->lastBlockPosition) {
				unsigned int end = start + this->lastBlockPosition;
				for (unsigned int i = start + this->firstBlockPosition + 1u; i < end; i++) {
					masks[i] = 0;
				}
				masks[end] &= this->lastBlock;
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
