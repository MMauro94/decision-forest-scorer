#ifndef FOREST_TREE_EVALUATOR_TREE_H
#define FOREST_TREE_EVALUATOR_TREE_H

#include <memory>
#include <utility>
#include <vector>
#include <thread>
#include <algorithm>
#include "Config.h"

class InternalNode;

/**
 * A node in the decision tree
 */
class Node {

	private:
		int _treeIndex = 0;
	public:
		/**
		 * Scores the given document
		 */
		[[nodiscard]] virtual double score(const std::vector<double> &element) const = 0;

		/**
		 * @return the number of leafs in this sub-tree
		 */
		[[nodiscard]] virtual unsigned int numberOfLeaves() const = 0;

		/**
		 * The index of the decision tree in the forest
		 */
		[[nodiscard]] int getTreeIndex() const {
			return this->_treeIndex;
		}

		/**
		 * Changes the index of the decision tree in the forest
		 */
		virtual void setTreeIndex(int treeIndex) {
			this->_treeIndex = treeIndex;
		}

		/**
		 * Counts the leafs until the given node is found, traversing the tree in pre-order vision.
		 *
		 * @param node The node to find
		 * @param found A boolean that will be set to true iff node has been found
		 */
		[[nodiscard]] virtual int countLeavesUntil(const std::shared_ptr<InternalNode> &node, bool *found) const = 0;

		/**
		 * Adds to leafScores all the scores in the leafs.
		 */
		virtual void fillLeafScores(std::vector<double> &leafScores) const = 0;

		/**
		 * The maximum index of a feature
		 */
		[[nodiscard]] virtual unsigned int maxFeatureIndex() const = 0;
};

/**
 * An internal node (i.e. not a leaf) in the decision tree
 */
class InternalNode : public Node {
	public:
		const unsigned int splittingFeatureIndex;
		const double splittingThreshold;
		std::shared_ptr<Node> leftNode;
		std::shared_ptr<Node> rightNode;

		InternalNode(
				unsigned int splittingFeatureIndex,
				double splittingThreshold,
				std::shared_ptr<Node> leftNode,
				std::shared_ptr<Node> rightNode
		) : splittingFeatureIndex(splittingFeatureIndex), splittingThreshold(splittingThreshold),
			leftNode(std::move(leftNode)), rightNode(std::move(rightNode)) {}

		[[nodiscard]] double score(const std::vector<double> &element) const override {
			if (element[this->splittingFeatureIndex] <= this->splittingThreshold) {
				return this->leftNode->score(element);
			} else {
				return this->rightNode->score(element);
			}
		}

		[[nodiscard]] unsigned int numberOfLeaves() const override {
			return this->leftNode->numberOfLeaves() + this->rightNode->numberOfLeaves();
		}

		void setTreeIndex(int treeIndex) override {
			Node::setTreeIndex(treeIndex);
			this->leftNode->setTreeIndex(treeIndex);
			this->rightNode->setTreeIndex(treeIndex);
		}

		[[nodiscard]] unsigned int maxFeatureIndex() const override {
			unsigned int ret = this->splittingFeatureIndex;
			ret = std::max(ret, this->leftNode->maxFeatureIndex());
			ret = std::max(ret, this->rightNode->maxFeatureIndex());
			return ret;
		}

		[[nodiscard]] int countLeavesUntil(const std::shared_ptr<InternalNode> &node, bool *found) const override {
			int ret = 0;
			if (this == node.get()) {
				*found = true;
			} else {
				ret += this->leftNode->countLeavesUntil(node, found);
				if (!*found) {
					ret += this->rightNode->countLeavesUntil(node, found);
				}
			}
			return ret;
		}

		void fillLeafScores(std::vector<double> &leafScores) const override {
			this->leftNode->fillLeafScores(leafScores);
			this->rightNode->fillLeafScores(leafScores);
		}
};

/**
 * A leaf in the decision tree
 */
class Leaf : public Node {
	private:
		const double _score;
	public:
		explicit Leaf(double score) : _score(score) {}

		[[nodiscard]] double score(const std::vector<double> &element) const override {
			return this->_score;
		}

		[[nodiscard]] unsigned int numberOfLeaves() const override {
			return 1;
		}

		int countLeavesUntil(const std::shared_ptr<InternalNode> &node, bool *found) const override {
			return 1;
		}

		void fillLeafScores(std::vector<double> &leafScores) const override {
			leafScores.push_back(this->_score);
		}

		[[nodiscard]] unsigned int maxFeatureIndex() const override {
			return 0;
		}
};

/**
 * The decision tree
 */
class Tree {
	private:
		std::vector<double> leafScores;
		unsigned int leafsCount;
		unsigned int _treeIndex = 0;
	public:
		std::shared_ptr<InternalNode> root;

		explicit Tree(std::shared_ptr<InternalNode> root) : root(std::move(root)) {
			this->leafsCount = this->root->numberOfLeaves();
			this->root->fillLeafScores(this->leafScores);
		}

		/**
		 * Scores the given document
		 */
		[[nodiscard]] double score(const std::vector<double> &element) const {
			return this->root->score(element);
		}

		/**
		 * The number of leaves in this tree
		 */
		[[nodiscard]] unsigned int numberOfLeaves() const {
			return this->leafsCount;
		}

		/**
		 * The maximum index of a feature
		 */
		[[nodiscard]] unsigned int maxFeatureIndex() const {
			return this->root->maxFeatureIndex();
		}

		/**
		 * The index of this tree inside the forest
		 */
		[[nodiscard]] unsigned int getTreeIndex() const {
			return this->_treeIndex;
		}

		/**
		 * Changes the index of this tree inside the forest
		 */
		void setTreeIndex(unsigned int treeIndex) {
			this->_treeIndex = treeIndex;
			this->root->setTreeIndex(treeIndex);
		}

		/**
		 * Counts the leafs until the given node is found, traversing the tree in pre-order vision.
		 *
		 * @param node The node to find
		 * @param found A boolean that will be set to true iff node has been found
		 */
		[[nodiscard]] int countLeavesUntil(const std::shared_ptr<InternalNode> &node) const {
			bool found = false;
			int ret = this->root->countLeavesUntil(node, &found);
			if (!found) {
				throw std::logic_error("node not found");
			}
			return ret;
		}

		/**
		 * The score of the leaf with the given index
		 */
		[[nodiscard]] double scoreByLeafIndex(unsigned long leafIndex) const {
			return this->leafScores[leafIndex];
		}
};

/**
 * The decision forest
 */
class Forest {
	private:
		unsigned int _maximumNumberOfLeafs;

		void computeMaximumNumberOfLeafs() {
			this->_maximumNumberOfLeafs = 0u;
			for (auto &tree : this->trees) {
				this->_maximumNumberOfLeafs = std::max(this->_maximumNumberOfLeafs, tree.numberOfLeaves());
			}
		}

	public:
		/**
		 * The trees inside this forest
		 */
		std::vector<Tree> trees;

		explicit Forest(std::vector<Tree> &trees) : trees(trees) {
			for (unsigned int index = 0, size = trees.size(); index < size; ++index) {
				trees[index].setTreeIndex(index);
			}
			this->computeMaximumNumberOfLeafs();
		}

		/**
		 * Divides the given trees inside various forests, according to the given config
		 */
		template<typename Scorer>
		static std::vector<std::shared_ptr<Forest>> buildForests(const Config<Scorer> &config, const std::vector<Tree> &trees) {
			const unsigned int threads = config.parallel_forests ? config.number_of_threads : 1u;

			std::vector<std::vector<Tree>> almostForests(threads);
			for (unsigned long i = 0; i < trees.size(); i++) {
				almostForests[i % threads].push_back(trees[i]);
			}
			std::vector<std::shared_ptr<Forest>> ret;
			ret.reserve(almostForests.size());
			for (auto &f : almostForests) {
				ret.push_back(std::make_shared<Forest>(f));
			}
			return ret;
		}

		/**
		 * The maximum index of a feature
		 */
		unsigned int maximumFeatureIndex() {
			unsigned int maxFeatureIndex = 0;
			for (auto &tree : this->trees) {
				maxFeatureIndex = std::max(maxFeatureIndex, tree.maxFeatureIndex());
			}
			return maxFeatureIndex;
		}

		/**
		 * The maximum number of leaves in each tree
		 */
		[[nodiscard]] unsigned int maximumNumberOfLeaves() const {
			return this->_maximumNumberOfLeafs;
		}

		/**
		 * Scores the given document
		 */
		[[nodiscard]] double score(const std::vector<double> &element) const {
			double score = 0;
			for (auto &tree : this->trees) {
				score += tree.score(element);
			}
			return score;
		}
};


#endif //FOREST_TREE_EVALUATOR_TREE_H
