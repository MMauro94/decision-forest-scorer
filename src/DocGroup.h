//
// Created by molin on 28/12/2019.
//

#ifndef FOREST_TREE_EVALUATOR_DOCGROUP_H
#define FOREST_TREE_EVALUATOR_DOCGROUP_H

#include <utility>
#include <vector>
#include "SIMDDoubleGroup.h"


class SingleDocument {
	public:
		std::vector<double> features;

		explicit SingleDocument(std::vector<double> features) : features(std::move(features)) {}

		[[nodiscard]] static int numberOfDocuments() {
			return 1;
		}

		static std::vector<SingleDocument> create(const std::vector<std::vector<double>> &documents) {
			std::vector<SingleDocument> ret;
			ret.reserve(documents.size());
			for (auto &doc : documents) {
				ret.emplace_back(doc);
			}
			return ret;
		}
};

class SIMDDocumentGroup : public SIMDDoubleGroup {
	public:
		SIMDDocumentGroup() = default;

		SIMDDocumentGroup(const std::vector<double> &doc1, const std::vector<double> &doc2,
						  const std::vector<double> &doc3, const std::vector<double> &doc4,
						  const std::vector<double> &doc5, const std::vector<double> &doc6,
						  const std::vector<double> &doc7, const std::vector<double> &doc8) : SIMDDoubleGroup(doc1,
																											  doc2,
																											  doc3,
																											  doc4,
																											  doc5,
																											  doc6,
																											  doc7,
																											  doc8) {}

		[[nodiscard]] static int numberOfDocuments() {
			return 8;
		}

		static std::vector<SIMDDocumentGroup> create(const std::vector<std::vector<double>> &documents) {
			return SIMDDoubleGroup::groupByEight<SIMDDocumentGroup>(documents);
		}
};

template<int DOCS>
class MultiSIMDDocumentGroup {
		/**
		 * This vector is organized as following:
		 * one feature after another. Each feature has DOCS double, represented with DOCS/8 __m512d
		 */
		std::vector<__m512d> features;
	public:
		[[nodiscard]] static int numberOfDocuments() {
			return DOCS;
		}

		/**
		 * Gets a feature of 8 different documents.
		 */
		__m512d getGroup(int docGroup, int feature) const {
			return this->features[feature * DOCS / 8 + docGroup];
		}

		static std::vector<MultiSIMDDocumentGroup<DOCS>> create(const std::vector<std::vector<double>> &documents) {
			std::vector<MultiSIMDDocumentGroup<DOCS>> ret;
			unsigned int features = documents[0].size();
			unsigned long max = documents.size() - 1;

			for (unsigned long i = 0; i < documents.size(); i += DOCS) {
				MultiSIMDDocumentGroup<DOCS> group;
				for (int f = 0; f < features; f++) {
					//Docs should be put there in reverse order
					for (int j = DOCS - 8; j >= 0; j -= 8) {
						group.features.emplace_back(_mm512_set_pd(
								documents[std::min(i + j + 7, max)][f],
								documents[std::min(i + j + 6, max)][f],
								documents[std::min(i + j + 5, max)][f],
								documents[std::min(i + j + 4, max)][f],
								documents[std::min(i + j + 3, max)][f],
								documents[std::min(i + j + 2, max)][f],
								documents[std::min(i + j + 1, max)][f],
								documents[std::min(i + j + 0, max)][f]
						));
					}
				}
				ret.emplace_back(group);
			}
			return ret;
		}
};

#endif //FOREST_TREE_EVALUATOR_DOCGROUP_H
