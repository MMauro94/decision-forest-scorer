//
// Created by molin on 28/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_RESULTMASK_H
#define FOREST_TREE_EVALUATOR_RESULTMASK_H


#include <vector>
#include <stdint-gcc.h>
#include <mutex>
#include <deque>
#include <atomic>
#include "config.h"
#include "Tree.h"
#include "Epitome.h"

class ResultMask {
	private:
		std::shared_ptr<Forest> _forest;
		std::vector<MaskType> masks;

	public:

		explicit ResultMask(std::shared_ptr<Forest> forest) : _forest(std::move(forest)), masks(this->_forest->trees.size()) {
#pragma omp parallel for if(PARALLEL_INIT) default(none)
			for (unsigned long i = 0; i < this->_forest->trees.size(); i++) {
				auto &t = this->_forest->trees[i];
				int maskByteSize = t.numberOfLeafs() / 8 + 1;
				auto mask = MaskType(maskByteSize);
				for (int j = 0; j < maskByteSize; j++) {
					mask[j] = 255;
				}
				this->masks[i].swap(mask);
			}
		}

		void applyMask(const Epitome &mask, const unsigned int treeIndex) {
			mask.performAnd(this->masks[treeIndex]);
		}

		[[nodiscard]] double computeScore() const {
			double score = 0.0;
#pragma omp parallel for if(PARALLEL_SCORE) default(none) reduction(+:score)
			for (unsigned long i = 0; i < this->_forest->trees.size(); i++) {
				auto leafIndex = firstOne(this->masks[i]);
				auto &tree = this->_forest->trees[i];
				score += tree.scoreByLeafIndex(leafIndex);
			}
			return score;
		}

	private:

		static unsigned int firstOne(const MaskType &vector) {
			int ret = 0;
			int i = 0;
			while (vector[i] == 0) {
				i++;
				ret += 8;
			}
			int j = 0;
			while ((vector[i] & (1u << (8u - j - 1u))) == 0) {
				j++;
				ret++;
			}
			return ret;
		}

};

#endif //FOREST_TREE_EVALUATOR_RESULTMASK_H
