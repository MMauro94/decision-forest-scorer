#include <memory>
#include "../Tree.h"
#include "../FeaturesMap.h"

int main() {
    /*
     * 	       A
     * 	   B       C
     * 	 D   E   F   G
     * 	H I     N O P Q
     */
    auto h = std::make_shared<Leaf>(0.1);
    auto i = std::make_shared<Leaf>(0.2);
    auto n = std::make_shared<Leaf>(0.3);
    auto o = std::make_shared<Leaf>(0.4);
    auto p = std::make_shared<Leaf>(0.5);
    auto q = std::make_shared<Leaf>(0.6);

    auto d = std::make_shared<InternalNode>(0, 1.1, h, i);
    auto e = std::make_shared<Leaf>(0.7);
    auto f = std::make_shared<InternalNode>(2, 1.2, n, o);
    auto g = std::make_shared<InternalNode>(1, 1.3, p, q);

    auto b = std::make_shared<InternalNode>(0, 1.4, d, e);
    auto c = std::make_shared<InternalNode>(1, 1.5, f, g);

    auto a = std::make_shared<InternalNode>(2, 1.6, b, c);

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