//
// Created by MMarco on 23/10/2019.
//

#ifndef FOREST_TREE_EVALUATOR_TREE_H
#define FOREST_TREE_EVALUATOR_TREE_H

#include <memory>
#include <utility>
#include <vector>

class Node {
	public:
		virtual double score(std::vector<double> &element) = 0;
};

class InternalNode : Node {

	private:
		int featureIndex;
		double referenceValue;
		std::shared_ptr<Node> left;
		std::shared_ptr<Node> right;

	public:
		InternalNode(
				int featureIndex,
				double referenceValue,
				std::shared_ptr<Node> left,
				std::shared_ptr<Node> right
		) : featureIndex(featureIndex), referenceValue(referenceValue), left(std::move(left)), right(std::move(right)) {
		}

		double score(std::vector<double> &element) override {
			if (element[this->featureIndex] <= referenceValue) {
				return this->left->score(element);
			} else {
				return this->right->score(element);
			}
		}
};

class Leaf : Node {
	private:
		double result;
	public:
		explicit Leaf(double result) : result(result) {}

		double score(std::vector<double> &element) override {
			return this->result;
		}
};

class Tree {
	private:
		std::shared_ptr<InternalNode> root;

	public:
		double score(std::vector<double> &element) {
			return this->root->score(element);
		}
};


#endif //FOREST_TREE_EVALUATOR_TREE_H
