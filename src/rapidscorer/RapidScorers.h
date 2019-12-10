//
// Created by molin on 11/11/2019.
//

#ifndef FOREST_TREE_EVALUATOR_RAPIDSCORERE_H
#define FOREST_TREE_EVALUATOR_RAPIDSCORERE_H

#include <memory>
#include <algorithm>
#include "../Tree.h"
#include "../EqNode.h"
#include "../ResultMask.h"
#include "LinearizedRapidScorer.h"


#if LINEARIZE_EQ_NODES
typedef LinearizedRapidScorer RapidScorer;
#else
ftypedef EqNodesRapidScorer RapidScorer;
#endif

class RapidScorers {
		std::vector<RapidScorer> scorers;

	public:
		explicit RapidScorers(std::vector<std::shared_ptr<Forest>> &forests) {
			for (auto &forest : forests) {
				scorers.emplace_back(forest);
			}
		}

		[[nodiscard]] double score(const std::vector<double> &document) const {
			double score = 0.0;
#pragma omp parallel for if(PARALLEL_FORESTS) default(none) shared(document) reduction(+:score)
			for (unsigned long i = 0; i < this->scorers.size(); i++) { // NOLINT(modernize-loop-convert)
				score += this->scorers[i].score(document);
			}
			return score;
		}
};


#endif //FOREST_TREE_EVALUATOR_RAPIDSCORERE_H
