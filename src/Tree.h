#ifndef FOREST_TREE_EVALUATOR_TREE_H
#define FOREST_TREE_EVALUATOR_TREE_H

#include <memory>
#include <utility>
#include <vector>

class InternalNode;

class Node {
	private:
		int _treeIndex = 0;
	public:
		[[nodiscard]] virtual double score(const std::vector<double> &element) const = 0;

		[[nodiscard]] virtual int numberOfLeafs() const = 0;

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

		[[nodiscard]] int numberOfLeafs() const override {
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

		[[nodiscard]] int numberOfLeafs() const override {
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
		int leafsCount;
		int _treeIndex = 0;
	public:
		std::shared_ptr<InternalNode> root;

		explicit Tree(std::shared_ptr<InternalNode> root) : root(std::move(root)) {
			this->leafsCount = this->root->numberOfLeafs();
			this->root->fillLeafScores(this->leafScores);
		}

		[[nodiscard]] double score(const std::vector<double> &element) const {
			return this->root->score(element);
		}

		[[nodiscard]] int numberOfLeafs() const {
			return this->leafsCount;
		}

		void fillNodesByFeature(std::vector<std::vector<std::shared_ptr<InternalNode>>> &nodes) const {
			nodes[this->root->splittingFeatureIndex].push_back(this->root);
			this->root->fillNodesByFeature(nodes);
		}

		[[nodiscard]] int getTreeIndex() const {
			return this->_treeIndex;
		}

		void setTreeIndex(int treeIndex) {
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

		[[nodiscard]] double scoreByLeafIndex(unsigned int leafIndex) const {
			return this->leafScores[leafIndex];
		}
};

class Forest {
	public:
		std::vector<Tree> trees;

		explicit Forest(std::vector<Tree> &trees) : trees(trees) {
			for (int index = 0, size = trees.size(); index < size; ++index) {
				trees[index].setTreeIndex(index);
			}
		}

		[[nodiscard]] int maximumNumberOfLeafs() const {
			int max = 0;
			for (auto &tree : this->trees) {
				max = std::max(max, tree.numberOfLeafs());
			}
			return max;
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
