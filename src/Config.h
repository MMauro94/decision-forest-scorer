//
// Created by MMarco on 05/11/2019.
//
#ifndef CONFIG
#define CONFIG

class SIMDRapidScorer;

template <typename Scorer>
class Config {
	public:
		const bool parallel_mask;
		const bool parallel_score;
		const bool parallel_forests;
		const bool parallel_documents;
		const int number_of_threads;

		Config(
				bool parallelMask,
				bool parallelScore,
				bool parallelForests,
				bool parallelDocuments,
				int numberOfThreads
		) : parallel_mask(parallelMask),
			parallel_score(parallelScore),
			parallel_forests(parallelForests),
			parallel_documents(parallelDocuments),
			number_of_threads(numberOfThreads) {
			if(std::is_same<Scorer, SIMDRapidScorer>::value && parallelMask) {
				throw "Unsupported SIMD + parallel mask";
			}
		}
};

#define PARALLEL_DOCUMENTS false

#define NUMBER_OF_THREADS 16

#endif