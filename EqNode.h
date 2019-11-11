//
// Created by molin on 11/11/2019.
//

#ifndef FOREST_TREE_EVALUATOR_EQNODE_H
#define FOREST_TREE_EVALUATOR_EQNODE_H

#include "Tree.h"
#include "Epitome.h"

class EqNode {

	public:
		unsigned int featureIndex;
		double featureThreshold;
		unsigned int treeIndex;
		Epitome epitome;

		EqNode(const std::shared_ptr<InternalNode> &node, const Epitome &epitome) : featureIndex(node->splittingFeatureIndex),
																			  featureThreshold(
																					  node->splittingThreshold),
																			  treeIndex(node->getTreeIndex()),
																			  epitome(epitome) {
		}

		bool operator<(const EqNode &rhs) const {
			if (featureIndex < rhs.featureIndex) return true;
			else if (featureIndex > rhs.featureIndex) return false;
			else return featureThreshold < rhs.featureThreshold;
		}

		bool operator>(const EqNode &rhs) const {
			return rhs < *this;
		}

		bool operator<=(const EqNode &rhs) const {
			return !(rhs < *this);
		}

		bool operator>=(const EqNode &rhs) const {
			return !(*this < rhs);
		}
};

#endif //FOREST_TREE_EVALUATOR_EQNODE_H
