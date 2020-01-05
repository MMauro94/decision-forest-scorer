#ifndef FOREST_TREE_EVALUATOR_RAPIDSCORERE_H
#define FOREST_TREE_EVALUATOR_RAPIDSCORERE_H

#include <memory>
#include <algorithm>
#include "../Config.h"
#include "../Tree.h"
#include "../EqNode.h"
#include "../ResultMask.h"

template<class RapidScorer>
class RapidScorers {
		Config<RapidScorer> config;
		std::vector<RapidScorer> scorers;

	public:
		explicit RapidScorers(const Config<RapidScorer> &config, const std::vector<std::shared_ptr<Forest>> &forests) : config(config) {
			for (auto &forest : forests) {
				scorers.emplace_back(config, forest);
			}
		}

		[[nodiscard]] std::vector<double> score(const typename RapidScorer::DocGroup &documents) const {
			std::vector<double> scores(RapidScorer::DocGroup::numberOfDocuments(), 0.0);
#pragma omp parallel for num_threads(this->config.number_of_threads) if(this->config.parallel_forests) default(none) shared(documents) shared(scores)
			for (unsigned long i = 0; i < this->scorers.size(); i++) { // NOLINT(modernize-loop-convert)
				std::vector<double> score = this->scorers[i].score(documents);
				for (unsigned int j = 0; j < RapidScorer::DocGroup::numberOfDocuments(); j++) {
#pragma omp atomic update
					scores[j] += score[j];
				}
			}
			return scores;
		}

};


#endif //FOREST_TREE_EVALUATOR_RAPIDSCORERE_H
