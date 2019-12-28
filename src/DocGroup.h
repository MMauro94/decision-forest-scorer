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

		static std::vector<SingleDocument> create(const std::vector<std::vector<double>>& documents) {
			std::vector<SingleDocument> ret;
			ret.reserve(documents.size());
			for(auto &doc : documents) {
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

		static std::vector<SIMDDocumentGroup> create(const std::vector<std::vector<double>>& documents) {
			return SIMDDoubleGroup::groupByEight<SIMDDocumentGroup>(documents);
		}
};

#endif //FOREST_TREE_EVALUATOR_DOCGROUP_H
