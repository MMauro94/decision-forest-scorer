#include <iostream>
#include "json.hpp"
#include "Tree.h"
#include "RapidScorer.h"
#include <iostream>
#include <fstream>
#include <sstream>

#define TEST_EQUALITY_THRESHOLD 0.00001
std::string DOCUMENTS_ROOT = "documents";

std::shared_ptr<Node> parseNode(nlohmann::json json) {
	if (json.contains("split_feature")) {
		//Internal
		assert(json["decision_type"] == "<=");
		assert(json["default_left"] == true);
		return std::make_shared<InternalNode>(
				json["split_feature"],
				json["threshold"],
				parseNode(json["left_child"]),
				parseNode(json["right_child"])
		);
	} else {
		//Leaf
		return std::make_shared<Leaf>(json["leaf_value"]);
	}
}

Tree parseTree(nlohmann::json json) {
	auto root = parseNode(json["tree_structure"]);
	return Tree(std::dynamic_pointer_cast<InternalNode>(root));
}

std::shared_ptr<Forest> parseForest(const int fold) {
	std::ifstream file(DOCUMENTS_ROOT + "/Fold" + std::to_string(fold) + "/model.json");
	nlohmann::json json;
	file >> json;
	file.close();

	std::vector<Tree> trees;
	for (auto &tree : json["tree_info"]) {
		trees.push_back(parseTree(tree));
	}
	return std::make_shared<Forest>(trees);
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
			ret.push_back(value); //TODO use feature ID
			assert(featureId == ret.size());
		}
	}
	return ret;
}

std::vector<std::vector<double>> parseDocuments(const int fold) {
	std::ifstream file;
	file.open(DOCUMENTS_ROOT + "/Fold" + std::to_string(fold) + "/test.txt");
	std::string s;
	std::vector<std::vector<double>> ret;
	while (std::getline(file, s)) {
		ret.push_back(parseDocumentLine(s));
	}
	return ret;
}

std::vector<double> parseScores(const int fold) {
	std::ifstream file;
	file.open(DOCUMENTS_ROOT + "/Fold" + std::to_string(fold) + "/test_scores.txt");
	std::string s;
	std::vector<double> ret;
	while (std::getline(file, s)) {
		ret.push_back(std::stod(s));
	}
	return ret;
}


void testFold(const int fold) {
	auto f = parseForest(fold);

	const auto &doc = parseDocuments(fold);
	const auto &testScores = parseScores(fold);
	assert(doc.size() == testScores.size());

	RapidScorer scorer(f);
	for (int i = 0, max = doc.size(); i < max; i++) {
		//const double score = scorer.score(doc[i]);
		const double score = f->score(doc[i]);
		const double testScore = testScores[i];

		if (std::abs(score - testScore) > TEST_EQUALITY_THRESHOLD) {
			std::cout << "Test failed: Mismatch: expecting " << testScore << ", found " << score << std::endl;
			exit(1);
		}
	}
}

int main() {
	for (int i = 1; i <= 4; i++) {
		testFold(i);
	}

	return 0;
}