#ifndef FOREST_TREE_EVALUATOR_RAPIDSCORERE_H
#define FOREST_TREE_EVALUATOR_RAPIDSCORERE_H

#include <memory>
#include <algorithm>
#include "../config.h"
#include "../Tree.h"
#include "../EqNode.h"
#include "../ResultMask.h"
#include "../SIMDDoubleGroup.h"

template <class RapidScorer>
class RapidScorers {
		std::vector<RapidScorer> scorers;

	public:
		explicit RapidScorers(std::vector<std::shared_ptr<Forest>> &forests) {
			for (auto &forest : forests) {
				scorers.emplace_back(forest);
			}
		}

		[[nodiscard]] std::vector<double> score(const typename RapidScorer::DocGroup &documents) const {
			std::vector<double> scores(RapidScorer::DocGroup::numberOfDocuments(), 0.0);
#pragma omp parallel for num_threads(NUMBER_OF_THREADS) if(PARALLEL_FORESTS) default(none) shared(documents) shared(scores)
			for (unsigned long i = 0; i < this->scorers.size(); i++) { // NOLINT(modernize-loop-convert)
				std::vector<double> score = this->scorers[i].score(documents);
				for (unsigned int j = 0; j < RapidScorer::DocGroup::numberOfDocuments(); j++) {
#pragma omp atomic
					scores[j] += score[j];
				}
			}
			return scores;
		}

};


#endif //FOREST_TREE_EVALUATOR_RAPIDSCORERE_H
