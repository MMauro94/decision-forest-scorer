#ifndef FOREST_TREE_EVALUATOR_TREE_H
#define FOREST_TREE_EVALUATOR_TREE_H

#include <memory>
#include <utility>
#include <vector>
#include <thread>
#include <algorithm>
#include "Config.h"

class InternalNode;

class Node {
	private:
		int _treeIndex = 0;
	public:
		[[nodiscard]] virtual double score(const std::vector<double> &element) const = 0;

		[[nodiscard]] virtual unsigned int numberOfLeafs() const = 0;

		[[nodiscard]] int getTreeIndex() const {
			return this->_treeIndex;
		}

		virtual void setTreeIndex(int treeIndex) {
			this->_treeIndex = treeIndex;
		}

		[[nodiscard]] virtual int countLeafsUntil(const std::shared_ptr<InternalNode> &node, bool *found) const = 0;

		virtual void fillLeafScores(std::vector<double> &leafScores) const = 0;
};

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

		[[nodiscard]] unsigned int numberOfLeafs() const override {
			return this->leftNode->numberOfLeafs() + this->rightNode->numberOfLeafs();
		}

		void fillNodesByFeature(std::vector<std::vector<std::shared_ptr<InternalNode>>> &nodes) const {
			auto leftAsInternalNode = std::dynamic_pointer_cast<InternalNode>(this->leftNode);
			auto rightAsInternalNode = std::dynamic_pointer_cast<InternalNode>(this->rightNode);
			if (leftAsInternalNode != nullptr) {
				nodes[leftAsInternalNode->splittingFeatureIndex].push_back(leftAsInternalNode);
				leftAsInternalNode->fillNodesByFeature(nodes);
			}
			if (rightAsInternalNode != nullptr) {
				nodes[rightAsInternalNode->splittingFeatureIndex].push_back(rightAsInternalNode);
				rightAsInternalNode->fillNodesByFeature(nodes);
			}
		}

		void setTreeIndex(int treeIndex) override {
			Node::setTreeIndex(treeIndex);
			this->leftNode->setTreeIndex(treeIndex);
			this->rightNode->setTreeIndex(treeIndex);
		}

		[[nodiscard]] int countLeafsUntil(const std::shared_ptr<InternalNode> &node, bool *found) const override {
			int ret = 0;
			if (this == node.get()) {
				*found = true;
			} else {
				ret += this->leftNode->countLeafsUntil(node, found);
				if (!*found) {
					ret += this->rightNode->countLeafsUntil(node, found);
				}
			}
			return ret;
		}

		void fillLeafScores(std::vector<double> &leafScores) const override {
			this->leftNode->fillLeafScores(leafScores);
			this->rightNode->fillLeafScores(leafScores);
		}
};

class Leaf : public Node {
	private:
		const double _score;
	public:
		explicit Leaf(double score) : _score(score) {}

		[[nodiscard]] double score(const std::vector<double> &element) const override {
			return this->_score;
		}

		[[nodiscard]] unsigned int numberOfLeafs() const override {
			return 1;
		}

		int countLeafsUntil(const std::shared_ptr<InternalNode> &node, bool *found) const override {
			return 1;
		}

		void fillLeafScores(std::vector<double> &leafScores) const override {
			leafScores.push_back(this->_score);
		}
};

class Tree {
	private:
		std::vector<double> leafScores;
		unsigned int leafsCount;
		unsigned int _treeIndex = 0;
	public:
		std::shared_ptr<InternalNode> root;

		explicit Tree(std::shared_ptr<InternalNode> root) : root(std::move(root)) {
			this->leafsCount = this->root->numberOfLeafs();
			this->root->fillLeafScores(this->leafScores);
		}

		[[nodiscard]] double score(const std::vector<double> &element) const {
			return this->root->score(element);
		}

		[[nodiscard]] unsigned int numberOfLeafs() const {
			return this->leafsCount;
		}

		void fillNodesByFeature(std::vector<std::vector<std::shared_ptr<InternalNode>>> &nodes) const {
			nodes[this->root->splittingFeatureIndex].push_back(this->root);
			this->root->fillNodesByFeature(nodes);
		}

		[[nodiscard]] unsigned int getTreeIndex() const {
			return this->_treeIndex;
		}

		void setTreeIndex(unsigned int treeIndex) {
			this->_treeIndex = treeIndex;
			this->root->setTreeIndex(treeIndex);
		}

		[[nodiscard]] int countLeafsUntil(const std::shared_ptr<InternalNode> &node) const {
			bool found = false;
			int ret = this->root->countLeafsUntil(node, &found);
			if (!found) {
				throw std::logic_error("node not found");
			}
			return ret;
		}

		[[nodiscard]] double scoreByLeafIndex(unsigned long leafIndex) const {
			return this->leafScores[leafIndex];
		}
};

class Forest {
	private:
		unsigned int _maximumNumberOfLeafs;

		void computeMaximumNumberOfLeafs() {
			this->_maximumNumberOfLeafs = 0u;
			for (auto &tree : this->trees) {
				this->_maximumNumberOfLeafs = std::max(this->_maximumNumberOfLeafs, tree.numberOfLeafs());
			}
		}

	public:
		std::vector<Tree> trees;

		explicit Forest(std::vector<Tree> &trees) : trees(trees) {
			for (unsigned int index = 0, size = trees.size(); index < size; ++index) {
				trees[index].setTreeIndex(index);
			}
			this->computeMaximumNumberOfLeafs();
		}

		template <typename Scorer>
		static std::vector<std::shared_ptr<Forest>> buildForests(const Config<Scorer> &config, std::vector<Tree> &trees) {
			unsigned int threads = config.parallel_forests ? config.number_of_threads : 1;

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

		[[nodiscard]] unsigned int maximumNumberOfLeafs() const {
			return this->_maximumNumberOfLeafs;
		}

		[[nodiscard]] double score(const std::vector<double> &element) const {
			double score = 0;
			for (auto &tree : this->trees) {
				score += tree.score(element);
			}
			return score;
		}

		[[nodiscard]] std::vector<std::vector<std::shared_ptr<InternalNode>>>
		getNodesByFeature(int numberOfFeatures) const {
			auto nodes = std::vector<std::vector<std::shared_ptr<InternalNode>>>();
			for (int i = 0; i < numberOfFeatures; i++) {
				nodes.emplace_back();
			}
			for (auto &t:this->trees) {
				t.fillNodesByFeature(nodes);
			}
			return nodes;
		}
};


#endif //FOREST_TREE_EVALUATOR_TREE_H
