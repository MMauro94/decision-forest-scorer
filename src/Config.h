//
// Created by MMarco on 05/11/2019.
//
#ifndef CONFIG
#define CONFIG

#include <ostream>

class SIMDRapidScorer;

template<typename Scorer>
class Config {
	public:
		const bool parallel_mask;
		const bool parallel_score;
		const bool parallel_forests;
		const bool parallel_documents;
		const unsigned int number_of_threads;

		Config(
				bool parallelMask,
				bool parallelScore,
				bool parallelForests,
				bool parallelDocuments,
				unsigned int numberOfThreads
		) : parallel_mask(parallelMask),
			parallel_score(parallelScore),
			parallel_forests(parallelForests),
			parallel_documents(parallelDocuments),
			number_of_threads(numberOfThreads) {
			if (std::is_same<Scorer, SIMDRapidScorer>::value && parallelMask) {
				throw std::invalid_argument("Unsupported SIMD + parallel mask");
			}
		}

		friend std::ostream &operator<<(std::ostream &os, const Config &config) {
			os <<
			"scorer: " << typeid(Scorer).name() <<
			" parallel_mask: " << config.parallel_mask <<
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