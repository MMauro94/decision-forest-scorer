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

		Epitome(int leftOnes, int middleZeroes) {
			if (leftOnes < 0 || middleZeroes <= 0) {
				throw std::invalid_argument("invalid arguments");
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
			for (int i = this->firstBlockPosition + 1 + start; i < this->lastBlockPosition + start; i++) {
				masks[i] = 0;
			}
			masks[start + this->lastBlockPosition] &= this->lastBlock;
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
