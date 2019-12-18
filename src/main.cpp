#include <iostream>
#include "Tree.h"
#include "rapidscorer/RapidScorers.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "SIMDDoubleGroup.h"
#include "rapidscorer/SIMDRapidScorer.h"
#include <iostream>
#include <fstream>
#include <sstream>

#define TEST_EQUALITY_THRESHOLD 0.00001
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

std::vector<std::shared_ptr<Forest>> parseForests(const int fold) {
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

	std::cout << "Building forests...";
	const auto &vector = Forest::buildForests(trees);
	std::cout << "OK" << std::endl;

	return vector;
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

std::vector<std::vector<double>> parseDocuments(const int fold, const int max = -1) {
	std::ifstream file;
	file.open(DOCUMENTS_ROOT + "/Fold" + std::to_string(fold) + "/test.txt");
	std::string s;
	std::vector<std::vector<double>> ret;
	while (std::getline(file, s) && (max == -1 || ret.size() < max)) {
		ret.push_back(parseDocumentLine(s));
	}
	return ret;
}

std::vector<double> parseScores(const int fold, const int max = -1) {
	std::ifstream file;
	file.open(DOCUMENTS_ROOT + "/Fold" + std::to_string(fold) + "/test_scores.txt");
	std::string s;
	std::vector<double> ret;
	while (std::getline(file, s) && (max == -1 || ret.size() < max)) {
		ret.push_back(std::stod(s));
	}
	return ret;
}


long testFold(const int fold) {
	const int max = 10000;

	auto f = parseForests(fold);

	std::cout << "Parsing documents...";
	const auto &doc = parseDocuments(fold, max);
	const auto &docGroups = SIMDDoubleGroup::groupByEight(doc);
	std::cout << "OK" << std::endl;

	std::cout << "Parsing scores...";
	const auto &testScores = parseScores(fold, max);

	assert(doc.size() == testScores.size());
	std::cout << "OK" << std::endl;


	RapidScorers<SIMDRapidScorer> scorer(f);
	std::cout << "Starting scoring..." << std::endl;

	auto t1 = std::chrono::high_resolution_clock::now();
#pragma omp parallel for if(PARALLEL_DOCUMENTS) default(none), shared(scorer), shared(doc), shared(testScores), shared(std::cout), shared(docGroups)
	for (int i = 0; i < docGroups.size(); i++) {
		const auto score = scorer.score(docGroups[i]);
		//const double score = f->score(doc[i]);

		/*if (i % 1000 == 0) {
			auto t2 = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

			std::cout << "Done " << i << " documents in " << duration / 1000000000.0 << "s" << std::endl;
		}*/

		for (int j = 0; j < 8 && i * 8 + j < testScores.size(); j++) {
			const double testScore = testScores[i * 8 + j];
			if (std::abs(score[j] - testScore) > TEST_EQUALITY_THRESHOLD) {
				std::string out = "Test failed at " + std::to_string(i) +  "/" + std::to_string(docGroups.size()) + ": Mismatch: expecting " + std::to_string(testScore) + ", found " +
								  std::to_string(score[j]) + "\n";
				std::cout << out;
				//exit(1);
			}
		}
	}
	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

	std::cout << "Fold " << fold << " took " << duration / 1000000000.0 << "s" << std::endl;
	return duration;
}

int main() {
	std::cout.setf(std::ios::unitbuf);

	unsigned long tot = 0;
	for (int i = 1; i <= 1; i++) {
		tot += testFold(i);
	}
	std::cout << "All took " << tot / 1000000000.0 << "s" << std::endl;
	return 0;
}
/*
int main() {
	auto t1 = std::chrono::high_resolution_clock::now();

	std::cout << "Starting parsing model.json" << std::endl;
	auto filename = DOCUMENTS_ROOT + "/Fold" + std::to_string(2) + "/model.json";
	FILE* fp = std::fopen(filename.c_str(), "rb"); // non-Windows use "r"

	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document json;
	json.ParseStream(is);
	fclose(fp);

	for(auto &c : json["tree_info"].GetArray()) {
		auto &o = c["tree_structure"];
		std::cout << "no";
	}
}*/