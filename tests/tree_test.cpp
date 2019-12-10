#include <memory>
#include <random>
#include <chrono>
#include "../src/Tree.h"
#include "../src/rapidscorer/RapidScorers.h"

double dRand(double min, double max) {
	double d = (double) rand() / RAND_MAX;
	return min + d * (max - min);
}

double iRand(int min, int max) {
	return rand() % (max - min) + min;
}

std::vector<double> generateDocument(int size, int seed) {
	srand(seed);
	std::vector<double> ret = {};
	for (int i = 0; i < size; i++) {
		ret.push_back(dRand(0, 1));
	}
	return ret;
}

std::shared_ptr<InternalNode> generateNode(int numberOfLeaves, int maxFeatures) {
	std::shared_ptr<Node> left, right;
	if (numberOfLeaves <= 3) {
		left = std::make_shared<Leaf>(dRand(0, 1));
		right = std::make_shared<Leaf>(dRand(0, 1));
	} else {
		left = generateNode(numberOfLeaves / 2, maxFeatures);
		right = generateNode(numberOfLeaves / 2, maxFeatures);
	}
	return std::make_shared<InternalNode>(
			iRand(0, maxFeatures),
			dRand(0, 1),
			left,
			right
	);
}

Tree generateTree(int numberOfLeaves, int maxFeatures, int seed) {
	srand(seed);
	return Tree(generateNode(numberOfLeaves, maxFeatures));
}

#define N_TEST 100
#define N_DOCUMENTS 10
#define N_FEATURES 1000
#define N_LEAVES 400
#define N_TREES 1000

int main() {
	std::vector<Tree> trees = {};
	for (int num = 0; num < N_TREES; num++) {
		trees.push_back(generateTree(N_LEAVES, N_FEATURES, num));
	}
	auto forest = std::make_shared<Forest>(trees);
	std::cout << "Piantati" << std::endl;

	std::vector<std::vector<double>> documents = {};
	for (int num = 0; num < N_DOCUMENTS; num++) {
		documents.push_back(generateDocument(N_FEATURES, num));
	}
	std::cout << "Docs" << std::endl;

	auto features = RapidScorer(forest);
	std::cout << "Created feature map" << std::endl;

	//Performing a basic test to ensure that there are no errors
	std::cout << "Performing a test" << std::endl;
	for (auto &d : documents) {
		double baseScore = forest->score(d);
		double fastScore = features.score(d);
		//Avoiding floating point mismatchs
		if (std::abs(baseScore - fastScore) > 0.00001) {
			std::cout << "Test failed: Mismatch: expecting " << baseScore << ", found " << fastScore << std::endl;
			exit(1);
		}
	}
	std::cout << "Test passed!" << std::endl;

	std::cout << "Starting doc evaluation" << std::endl;

	auto start = std::chrono::high_resolution_clock::now();
	for (long test = 0; test < N_TEST; test++) {
		for (auto &d:documents) {
			double result = features.score(d);
		}
	}
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	std::cout << "Evaluating took " << duration.count() / 1000000.0 << " sec" << std::endl;

	return 0;
}