#ifndef FOREST_TREE_EVALUATOR_SIMDRAPIDSCORER_H
#define FOREST_TREE_EVALUATOR_SIMDRAPIDSCORER_H

#include <memory>
#include <algorithm>
#include "../Tree.h"
#include "../ResultMask.h"
#include "../DocGroup.h"
#include "../Config.h"
#include "../SIMDResultMask.h"
#include "MergedRapidScorer.h"

template<typename SIMDInfo>
class SingleFeatureSIMDRapidScorer;

template<typename SIMDInfo>
class SIMDRapidScorer {

		Config<SIMDRapidScorer<SIMDInfo>> config;
		std::shared_ptr<Forest> forest;
		std::vector<SingleFeatureSIMDRapidScorer<SIMDInfo>> featureScorers;

	public:
		typedef MultiSIMDDocumentGroup<SIMDInfo::groups> DocGroup;

		explicit SIMDRapidScorer(const Config<SIMDRapidScorer<SIMDInfo>> &config, std::shared_ptr<Forest> forest) : config(config), forest(std::move(forest)) {
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
class SingleFeatureSIMDRapidScorer : SingleFeatureMergedRapidScorer<typename SIMDInfo::base_type> {

		typedef typename SIMDInfo::mask_type simd_mask_type;
	public:
		typedef MultiSIMDDocumentGroup<SIMDInfo::groups> DocGroup;

		explicit SingleFeatureSIMDRapidScorer(const std::shared_ptr<Forest> &forest, unsigned int featureIndex)
				: SingleFeatureMergedRapidScorer<typename SIMDInfo::base_type>(forest, featureIndex) {
		}

		void score(const DocGroup &documents, SIMDResultMask<SIMDInfo> &result) const {
			int simdGroups = SIMDInfo::groups / 8;

			__m512d values[simdGroups];
			__mmask8 isLE[simdGroups];
			for (int g = 0; g < simdGroups; g++) {
				values[g] = documents.getGroup(g, this->featureIndex);
				isLE[g] = 0xFF;
			}

			unsigned int index = 0;
			unsigned int lastBlockIndex = 0;
			unsigned int end = this->featureThresholds.size();
			for (unsigned int thresholdIndex = 0; thresholdIndex < end; thresholdIndex++) {
				__m512d threshold = _mm512_set1_pd(this->featureThresholds[thresholdIndex]);

				simd_mask_type isLEMask = 0;
				for (int g = 0; g < simdGroups; g++) {
					// extract mask of comparison. 1 if the comparison is FALSE
					isLE[g] = _mm512_mask_cmp_pd_mask(isLE[g], values[g], threshold, _CMP_GT_OQ);
					isLEMask <<= 8;
					isLEMask |= isLE[g];
				}
				if (isLEMask == 0) {
					return;//All documents' values are bigger than the thresholds
				}

				unsigned int epitomesToEpitome = this->featureThresholdToOffset[thresholdIndex + 1];
				for (; index < epitomesToEpitome; index++) {
					if (this->lastBlockIsSameAsFirstBlock[index]) {
						result.applyMask(
								this->firstBlocks[index],
								this->firstBlockPositions[index],
								this->firstBlocks[index],
								this->firstBlockPositions[index],
								this->treeIndexes[index],
								isLEMask
						);
					} else {
						result.applyMask(
								this->firstBlocks[index],
								this->firstBlockPositions[index],
								this->lastBlocks[lastBlockIndex],
								this->lastBlockPositions[lastBlockIndex],
								this->treeIndexes[index],
								isLEMask
						);
						lastBlockIndex++;
					}
				}
			}
		}
};

#endif //FOREST_TREE_EVALUATOR_SIMDRAPIDSCORER_H
