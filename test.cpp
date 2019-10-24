#include <memory>
#include "Tree.h"
#include "FeaturesMap.h"

int main() {
	/*
	 * 	       A
	 * 	   B       C
	 * 	 D   E   F   G
	 * 	H I L M N O P Q
	 */
	auto h = std::make_shared<Leaf>(0.1);
	auto i = std::make_shared<Leaf>(0.2);
	auto l = std::make_shared<Leaf>(0.3);
	auto m = std::make_shared<Leaf>(0.4);
	auto n = std::make_shared<Leaf>(0.5);
	auto o = std::make_shared<Leaf>(0.6);
	auto p = std::make_shared<Leaf>(0.7);
	auto q = std::make_shared<Leaf>(0.8);

	auto d = std::make_shared<InternalNode>(0, 0.5, h, i);
	auto e = std::make_shared<InternalNode>(1, 0.6, l, m);
	auto f = std::make_shared<InternalNode>(2, 0.7, n, o);
	auto g = std::make_shared<InternalNode>(1, 0.9, p, q);

	auto b = std::make_shared<InternalNode>(0, 0.3, d, e);
	auto c = std::make_shared<InternalNode>(1, 0.2, f, g);

	auto a = std::make_shared<InternalNode>(2, 0.3, b, c);

	auto tree = Tree(a);
	std::vector<Tree> trees = {tree};
	auto forest = Forest(trees);
	printf("Leaf count = %d\n", tree.numberOfLeafs());


	std::vector<double> f1 = {0.5, 0.5, 0.5};

	printf("Score is %f\n", tree.score(f1));


	FeatureEvaluator(forest, 0);
	FeatureEvaluator(forest, 1);
	FeatureEvaluator(forest, 2);
	return 0;
}