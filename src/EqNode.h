//
// Created by molin on 11/11/2019.
//

#ifndef FOREST_TREE_EVALUATOR_EQNODE_H
#define FOREST_TREE_EVALUATOR_EQNODE_H

#include "Tree.h"
#include "Epitome.h"

/**
 * A class that holds all the information required to update a ResultMask
 * with an Epitome. In particular, contains the Epitome, the index of the feature,
 * the index of the tree and the associated featureThreshold.
 */
template <typename Block>
class EqNode {

	public:
		unsigned int featureIndex;
		double featureThreshold;
		unsigned int treeIndex;
		Epitome<Block> epitome;

		EqNode(const std::shared_ptr<InternalNode> &node, const Epitome<Block> &epitome) : featureIndex(node->splittingFeatureIndex),
																						   featureThreshold(
																					  node->splittingThreshold),
																						   treeIndex(node->getTreeIndex()),
																						   epitome(epitome) {
		}

		/**
		 * A method to compare the EqNodes to sort them by feature and then threshold
		 */
		bool operator<(const EqNode<Block> &rhs) const {
			if (featureIndex < rhs.featureIndex) return true;
			else if (featureIndex > rhs.featureIndex) return false;
			else return featureThreshold < rhs.featureThreshold;
		}

		bool operator>(const EqNode<Block> &rhs) const {
			return rhs < *this;
		}

		bool operator<=(const EqNode<Block> &rhs) const {
			return !(rhs < *this);
		}

		bool operator>=(const EqNode<Block> &rhs) const {
			return !(*this < rhs);
		}
};

#endif //FOREST_TREE_EVALUATOR_EQNODE_H
