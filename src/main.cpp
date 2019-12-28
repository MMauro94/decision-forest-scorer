#include <iostream>
#include "Tree.h"
#include "rapidscorer/RapidScorers.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "SIMDDoubleGroup.h"
#include "rapidscorer/SIMDRapidScorer.h"
#include "rapidscorer/LinearizedRapidScorer.h"
#include "TestCase.h"
#include "SIMDInfo.h"
#include "rapidscorer/EqNodesRapidScorer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <omp.h>
#include <map>
#include <set>

std::string DOCUMENTS_ROOT = "documents";

template<class GheSboro>
std::shared_ptr<Node> parseNode(const GheSboro &json) {
	if (json.HasMember("split_feature")) {
		//Internal
		assert(json["decision_type"] == "<=");
		assert(json["default_left"] == true);
		return std::make_shared<InternalNode>(
				json["split_feature"].GetInt(),
				json["threshold"].GetDouble(),
				parseNode(json["left_child"]),
				parseNode(json["right_child"])
		);
	} else {
		//Leaf
		return std::make_shared<Leaf>(json["leaf_value"].GetDouble());
	}
}

template<class GheSboro>
Tree parseTree(const GheSboro &json) {
	auto root = parseNode(json["tree_structure"].GetObject());
	return Tree(std::dynamic_pointer_cast<InternalNode>(root));
}

std::vector<Tree> parseTrees(const unsigned int fold) {
	auto t1 = std::chrono::high_resolution_clock::now();

	std::cout << "Starting parsing model.json" << std::endl;
	auto filename = DOCUMENTS_ROOT + "/Fold" + std::to_string(fold) + "/model.json";
	FILE *fp = std::fopen(filename.c_str(), "rb"); // non-Windows use "r"

	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document json;
	json.ParseStream(is);
	fclose(fp);


	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
	std::cout << "model.json parsed, parsing trees, took " << duration / 1000000000.0 << "s" << std::endl;

	std::vector<Tree> trees;
	for (auto &tree : json["tree_info"].GetArray()) {
		trees.push_back(parseTree(tree));
	}
	auto t3 = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
	std::cout << "Trees parsed, took " << duration / 1000000000.0 << "s" << std::endl;

	return trees;
}

std::vector<double> parseDocumentLine(const std::string &line) {
	std::istringstream ss(line);
	std::string token;

	std::getline(ss, token, ' ');
	std::getline(ss, token, ' ');

	std::vector<double> ret;
	int featureId;
	double value;
	while (std::getline(ss, token, ' ')) {
		if (sscanf(token.c_str(), "%d:%lf", &featureId, &value) == 2) {
			ret.push_back(value);
			assert(featureId == ret.size());
		}
	}
	return ret;
}

std::vector<std::vector<double>> parseDocuments(const unsigned int fold, const unsigned long max = 0u) {
	std::cout << "Parsing documents... ";

	std::ifstream file;
	file.open(DOCUMENTS_ROOT + "/Fold" + std::to_string(fold) + "/test.txt");
	std::string s;
	std::vector<std::vector<double>> ret;
	while (std::getline(file, s) && (max == 0 || ret.size() < max)) {
		ret.push_back(parseDocumentLine(s));
	}
	std::cout << "OK" << std::endl;

	return ret;
}

std::vector<double> parseScores(const unsigned int fold, const unsigned long max = 0) {
	std::cout << "Parsing scores...";
	std::ifstream file;
	file.open(DOCUMENTS_ROOT + "/Fold" + std::to_string(fold) + "/test_scores.txt");
	std::string s;
	std::vector<double> ret;
	while (std::getline(file, s) && (max == 0 || ret.size() < max)) {
		ret.push_back(std::stod(s));
	}
	std::cout << "OK" << std::endl;
	return ret;
}

#define MAX_DOCUMENTS 10000
#define FOLD 1

template<typename Scorer>
std::vector<std::shared_ptr<Testable>> generateTestsParallelisms() {
	return {
			std::make_shared<TestCase<Scorer>>(Config<Scorer>::serial(), MAX_DOCUMENTS, FOLD),

			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelFeature(2), MAX_DOCUMENTS, FOLD),
			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelFeature(4), MAX_DOCUMENTS, FOLD),
			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelFeature(8), MAX_DOCUMENTS, FOLD),
			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelFeature(16), MAX_DOCUMENTS, FOLD),

			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelDocuments(2), MAX_DOCUMENTS, FOLD),
			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelDocuments(4), MAX_DOCUMENTS, FOLD),
			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelDocuments(8), MAX_DOCUMENTS, FOLD),
			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelDocuments(16), MAX_DOCUMENTS, FOLD),

			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelForest(2), MAX_DOCUMENTS, FOLD),
			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelForest(4), MAX_DOCUMENTS, FOLD),
			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelForest(8), MAX_DOCUMENTS, FOLD),
			std::make_shared<TestCase<Scorer>>(Config<Scorer>::parallelForest(16), MAX_DOCUMENTS, FOLD),
	};
}

template <typename T>
std::vector<T> flatten(const std::vector<std::vector<T>> &vector) {
	std::vector<T> ret;
	for(auto &v : vector) {
		ret.insert(ret.end(), v.begin(), v.end());
	}
	return ret;
}

const auto TESTS = flatten<std::shared_ptr<Testable>>({
		generateTestsParallelisms<LinearizedRapidScorer<uint8_t>>(),
		generateTestsParallelisms<LinearizedRapidScorer<uint16_t>>(),
		generateTestsParallelisms<LinearizedRapidScorer<uint32_t>>(),
		generateTestsParallelisms<LinearizedRapidScorer<uint64_t>>(),

		generateTestsParallelisms<EqNodesRapidScorer<uint8_t>>(),
		generateTestsParallelisms<EqNodesRapidScorer<uint16_t>>(),
		generateTestsParallelisms<EqNodesRapidScorer<uint32_t>>(),
		generateTestsParallelisms<EqNodesRapidScorer<uint64_t>>(),

		generateTestsParallelisms<SIMDRapidScorer<SIMD256Info>>(),
		generateTestsParallelisms<SIMDRapidScorer<SIMD512Info>>()
});


std::set<unsigned int> detectFolds() {
	std::set<unsigned int> folds;
	for (auto &test : TESTS) {
		folds.insert(test->fold);
	}
	return folds;
}

std::vector<std::shared_ptr<Testable>> testsForFold(unsigned int fold) {
	std::vector<std::shared_ptr<Testable>> tests;
	for (auto &test : TESTS) {
		if (test->fold == fold) {
			tests.push_back(test);
		}
	}
	return tests;
}

int main() {
	std::cout.setf(std::ios::unitbuf);

	std::cout << "Total tests: " << TESTS.size() << std::endl;

	for (auto &fold : detectFolds()) {
		std::cout << "TESTING FOLD " << fold << std::endl;

		const auto tests = testsForFold(fold);

		const unsigned long max_documents = std::max_element(tests.begin(), tests.end(), [](const std::shared_ptr<Testable> &t1, const std::shared_ptr<Testable> &t2) -> bool {
			return t1->max_documents < t2->max_documents;
		})->get()->max_documents;

		const auto &trees = parseTrees(fold);
		const auto &documents = parseDocuments(fold, max_documents);
		const auto &testScores = parseScores(fold, max_documents);

		assert(documents.size() == testScores.size());


		for (auto &test : tests) {
			test->test(trees, documents, testScores);
		}
	}

	return 0;
}