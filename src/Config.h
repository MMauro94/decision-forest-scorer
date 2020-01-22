//
// Created by MMarco on 05/11/2019.
//
#ifndef CONFIG
#define CONFIG

#include <ostream>

template<typename Scorer>
class Config {
	public:
		/**
		 * Whether to evaluate the features in parallel inside the scorers
		 */
		const bool parallel_features;
		/**
		 * Whether to compute the scores in parallel
		 */
		const bool parallel_score;
		/**
		 * Whether to divide the forest in sub-forests, and evaluate them in parallel
		 */
		const bool parallel_forests;
		/**
		 * Whether to evaluate more documents in parallel
		 */
		const bool parallel_documents;
		/**
		 * The number of threads to use in the parallel parts of the program
		 */
		const unsigned int number_of_threads;

		Config(
				bool parallelMask,
				bool parallelScore,
				bool parallelForests,
				bool parallelDocuments,
				unsigned int numberOfThreads
		) : parallel_features(parallelMask),
			parallel_score(parallelScore),
			parallel_forests(parallelForests),
			parallel_documents(parallelDocuments),
			number_of_threads(numberOfThreads) {
		}

		friend std::ostream &operator<<(std::ostream &os, const Config &config) {
			os <<
			"scorer: " << typeid(Scorer).name() <<
			" parallel_features: " << config.parallel_features <<
			" parallel_score: " << config.parallel_score <<
			" parallel_forests: " << config.parallel_forests <<
			" parallel_documents: " << config.parallel_documents <<
			" number_of_threads: " << config.number_of_threads;
			return os;
		}

		static Config<Scorer> serial() {
			return Config(false, false, false, false, 1);
		}

		static Config<Scorer> parallelFeature(unsigned int number_of_threads) {
			return Config(true, true, false, false, number_of_threads);
		}

		static Config<Scorer> parallelForest(unsigned int number_of_threads) {
			return Config(false, false, true, false, number_of_threads);
		}

		static Config<Scorer> parallelDocuments(unsigned int number_of_threads) {
			return Config(false, false, false, true, number_of_threads);
		}
};




#endif