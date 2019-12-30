#ifndef FOREST_TREE_EVALUATOR_SIMD2RAPIDSCORER_H
#define FOREST_TREE_EVALUATOR_SIMD2RAPIDSCORER_H

#include <memory>
#include <algorithm>
#include "../Tree.h"
#include "../ResultMask.h"
#include "../DocGroup.h"
#include "../Config.h"
#include "../SIMDResultMask.h"
#include "MergedRapidScorer.h"

template<typename SIMDInfo>
class SingleFeatureSIMDRapidScorer2;

template<typename SIMDInfo>
class SIMDRapidScorer2 {

		Config<SIMDRapidScorer2<SIMDInfo>> config;
		std::shared_ptr<Forest> forest;
		std::vector<SingleFeatureSIMDRapidScorer2<SIMDInfo>> featureScorers;

	public:
		typedef SIMDDocumentGroup DocGroup;

		explicit SIMDRapidScorer2(const Config<SIMDRapidScorer2<SIMDInfo>> &config, std::shared_ptr<Forest> forest) : config(config), forest(std::move(forest)) {
			auto featuresCount = this->forest->maximumFeatureIndex();
			for (unsigned int i = 0; i <= featuresCount; i++) {
				this->featureScorers.emplace_back(this->forest, i);
			}
		}

		[[nodiscard]] std::vector<double> score(const DocGroup &document) const {
			SIMDResultMask<SIMDInfo> result(this->forest);

			unsigned long max = this->featureScorers.size();
#pragma omp parallel for num_threads(this->config.number_of_threads) if(this->config.parallel_mask) default(none) shared(result) shared(document) shared(max)
			for (unsigned long featureIndex = 0; featureIndex < max; featureIndex++) {
				featureScorers[featureIndex].score(document, result);
			}

			return {result.computeScore(this->config)};
		}
};

template<typename SIMDInfo>
class SingleFeatureSIMDRapidScorer2 : SingleFeatureMergedRapidScorer<typename SIMDInfo::base_type> {

	public:
		typedef SIMDDocumentGroup DocGroup;

		explicit SingleFeatureSIMDRapidScorer2(const std::shared_ptr<Forest> &forest, unsigned int featureIndex)
				: SingleFeatureMergedRapidScorer<typename SIMDInfo::base_type>(forest, featureIndex, false) {
		}

		void score(const DocGroup &documents, SIMDResultMask<SIMDInfo> &result) const {
			__m512d value = documents.get(this->featureIndex);
			unsigned int end = this->featureThresholds.size();

			__mmask8 isLE = 0xFF;
			unsigned index = 0;
			unsigned lastBlockIndex = 0;
			for (unsigned int thresholdIndex = 0; thresholdIndex < end && isLE > 0; thresholdIndex++) {
				isLE = _mm512_mask_cmp_pd_mask(isLE, value, _mm512_set1_pd(this->featureThresholds[thresholdIndex]), _CMP_GT_OQ);
				// extract mask of comparison 1 if the comparison is FALSE
				unsigned int epitomesToEpitome = this->featureThresholdToOffset[thresholdIndex + 1];
				for (; index < epitomesToEpitome; index++) {
					if (this->lastBlockIsSameAsFirstBlock[index]) {
						result.applyMask(
								this->firstBlocks[index],
								this->firstBlockPositions[index],
								this->firstBlocks[index],
								this->firstBlockPositions[index],
								this->treeIndexes[index],
								isLE
						);
					} else {
						result.applyMask(
								this->firstBlocks[index],
								this->firstBlockPositions[index],
								this->lastBlocks[lastBlockIndex],
								this->lastBlockPositions[lastBlockIndex],
								this->treeIndexes[index],
								isLE
						);
						lastBlockIndex++;
					}
				}
			}
		}
};

#endif //FOREST_TREE_EVALUATOR_SIMD2RAPIDSCORER_H
