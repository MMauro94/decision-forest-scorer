//
// Created by molin on 28/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_RESULTMASK_H
#define FOREST_TREE_EVALUATOR_RESULTMASK_H


#include <vector>
#include <mutex>
#include <deque>
#include <atomic>
#include <strings.h>
#include "config.h"
#include "Tree.h"
#include "Epitome.h"

class ResultMask {
	private:
		std::shared_ptr<Forest> _forest;
		unsigned int masksPerTree;
		std::vector<MaskType> results;

	public:

		explicit ResultMask(std::shared_ptr<Forest> forest) :
				_forest(std::move(forest)),
				masksPerTree(this->_forest->maximumNumberOfLeafs() / MASK_SIZE + 1),
				results(this->_forest->trees.size() * this->masksPerTree, -1) {}

		void applyMask(const Epitome<BLOCK> &epitome, const unsigned int treeIndex) {
			epitome.performAnd(this->results, treeIndex, this->masksPerTree);
		}

		[[nodiscard]] double computeScore() const {
			double score = 0.0;
#pragma omp parallel for num_threads(NUMBER_OF_THREADS) if(PARALLEL_SCORE) default(none) reduction(+:score)
			for (unsigned long i = 0; i < this->_forest->trees.size(); i++) {
				auto leafIndex = this->firstOne(i);
				auto &tree = this->_forest->trees[i];
				score += tree.scoreByLeafIndex(leafIndex);//TODO: capire se c'è un modo meglio di ottenere questo score
			}
			return score;
		}

	private:

		[[nodiscard]] unsigned long firstOne(unsigned long treeIndex) const {
			unsigned long mult = this->masksPerTree * treeIndex;

			unsigned long i = 0;
			while (this->results[mult + i] == 0) {
				i++;
			}
			auto mask = this->results[mult + i];


			unsigned int j = 0;
			for (; j < MASK_SIZE; j++) {
				if ((mask >> (MASK_SIZE - 1 - j)) % 2 == 1) break;
			}

			return i * MASK_SIZE + j;
		}

};

#endif //FOREST_TREE_EVALUATOR_RESULTMASK_H
