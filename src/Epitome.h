//
// Created by molin on 28/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_EPITOME_H
#define FOREST_TREE_EVALUATOR_EPITOME_H

#include <stdint-gcc.h>
#include <bits/exception.h>
#include <stdexcept>
#include <iostream>
#include <bitset>
#include <vector>
#include <atomic>
#include <deque>
#include "config.h"

class Epitome {
	private:
		uint8_t firstByte;
		uint8_t firstBytePosition;
		uint8_t lastByte;
		uint8_t lastBytePosition;

	public:

		Epitome(const Epitome &other) = default;

		Epitome(int leftOnes, int middleZeroes) {
			if (leftOnes < 0 || middleZeroes <= 0) {
				throw std::invalid_argument("invalid arguments");
			}
			this->firstByte = 0;
			for (unsigned int i = 0; i < leftOnes % 8; i++) {
				this->firstByte |= 1u << (8u - i - 1);
			}
			this->firstBytePosition = leftOnes / 8;

			this->lastByte = 0;
			if ((middleZeroes + leftOnes) % 8 > 0) {
				for (unsigned int i = (middleZeroes + leftOnes) % 8; i < 8; i++) {
					this->lastByte |= 1u << (8u - i - 1);
				}
			}
			this->lastBytePosition = (leftOnes + middleZeroes - 1) / 8;

			if (this->firstBytePosition == this->lastBytePosition) {
				this->firstByte |= this->lastByte;
				this->lastByte = this->firstByte;
			}
		}

		void performAnd(MaskType &vector) const {
			vector[this->firstBytePosition] &= this->firstByte;
			for (int i = this->firstBytePosition + 1; i < this->lastBytePosition; i++) {
				vector[i] = 0;
			}
			vector[this->lastBytePosition] &= this->lastByte;
		}

		friend std::ostream &operator<<(std::ostream &os, const Epitome &epitome) {
			os << epitome.toString(true);
			return os;
		}

		[[nodiscard]] std::string toString(bool separateBytes) const {
			std::string ret;

			for (int i = 0; i < this->firstBytePosition; i++) {
				ret += "11111111";
				if (separateBytes) {
					ret += " ";
				}
			}
			if (this->lastBytePosition != this->firstBytePosition) {
				std::bitset<8> fb(this->firstByte);
				ret += fb.to_string();
				if (separateBytes) {
					ret += " ";
				}
			}

			for (int i = this->firstBytePosition + 1; i < this->lastBytePosition; i++) {
				ret += "00000000";
				if (separateBytes) {
					ret += " ";
				}
			}
			std::bitset<8> lb(this->lastByte);
			ret += lb.to_string();
			return ret;
		}
};


#endif //FOREST_TREE_EVALUATOR_EPITOME_H
