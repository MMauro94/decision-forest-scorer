//
// Created by molin on 28/12/2019.
//

#ifndef FOREST_TREE_EVALUATOR_TESTCASE_H
#define FOREST_TREE_EVALUATOR_TESTCASE_H

#include <iostream>
#include <algorithm>
#include "Config.h"
#include "Tree.h"
#include "rapidscorer/RapidScorers.h"

#define TEST_EQUALITY_THRESHOLD 0.00001

class Testable {
	public:
		const unsigned long max_documents;
		const unsigned int fold;

		Testable(const unsigned long max_documents, const unsigned int fold) : max_documents(max_documents), fold(fold) {}

		virtual long test(const std::vector<Tree> &trees, const std::vector<std::vector<double>> &documents, const std::vector<double> &testScores) = 0;
};

template<typename Scorer>
class TestCase : public Testable {
		const Config<Scorer> config;

	public:
		TestCase(const Config<Scorer> &config, const unsigned long max_documents, const unsigned int fold) : config(config), Testable(max_documents, fold) {}

		long test(const std::vector<Tree> &trees, const std::vector<std::vector<double>> &documents, const std::vector<double> &testScores) override {
			std::cout << std::endl;
			std::cout << "STARTING TEST (max_documents=" << Testable::max_documents << ")" << std::endl;
			std::cout << "Config: " << this->config << std::endl;


			std::cout << "Building forests...";
			const auto &f = Forest::buildForests(config, trees);
			std::cout << "OK" << std::endl;


			std::cout << "Grouping documents...";
			const auto &docGroups = Scorer::DocGroup::create(documents);
			std::cout << "OK" << std::endl;


			std::cout << "Creating scorer...";
			RapidScorers<Scorer> scorer(config, f);
			std::cout << "OK" << std::endl;

			std::cout << "Starting scoring..." << std::endl;
			const unsigned long documents_per_group = Scorer::DocGroup::numberOfDocuments();
			const unsigned long total_groups = std::min(Testable::max_documents / documents_per_group + (Testable::max_documents % documents_per_group == 0 ? 0 : 1), docGroups.size());

			auto t1 = std::chrono::high_resolution_clock::now();
#pragma omp parallel for num_threads(config.number_of_threads) if(config.parallel_documents) default(none), shared(scorer), shared(testScores), shared(std::cout), shared(docGroups), shared(t1)
			for (int i = 0; i < total_groups; i++) {
				const auto score = scorer.score(docGroups[i]);

				/*const unsigned long done_documents = (i + 1) * documents_per_group;
				if (done_documents % (documents_per_group * 100) == 0 || i == docGroups.size() - 1) {
					auto t2 = std::chrono::high_resolution_clock::now();
					auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

					std::cout << "Done " << done_documents << " documents in " << duration / 1000000000.0 << "s (" << (duration / done_documents) << " ns)" << std::endl;
				}*/

				const unsigned long docs = i * documents_per_group;
				for (unsigned long j = 0; j < documents_per_group && docs + j < testScores.size(); j++) {
					const double testScore = testScores[docs + j];
					if (std::abs(score[j] - testScore) > TEST_EQUALITY_THRESHOLD) {
						std::string out = "Test failed at document " + std::to_string(docs + j) + "/" + std::to_string(total_groups * documents_per_group) +
										  ": Mismatch: expecting " + std::to_string(testScore) + ", found " +
										  std::to_string(score[j]) + "\n";
						std::cout << out;
						exit(1);
					}
				}
			}
			auto t2 = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

			std::cout << "END TEST! " << " Took " << duration / 1000000000.0 << "s (" << (duration / (total_groups * documents_per_group)) << " ns)" << std::endl;
			return duration;
		}

};

#endif //FOREST_TREE_EVALUATOR_TESTCASE_H
