//
// Created by molin on 10/12/2019.
//

#ifndef FOREST_TREE_EVALUATOR_SIMDDOUBLEGROUP_H
#define FOREST_TREE_EVALUATOR_SIMDDOUBLEGROUP_H

#include <immintrin.h>
#include <cassert>
#include <vector>

class SIMDDoubleGroup {
		std::vector<__m256d> values;

	public:
		SIMDDoubleGroup() {
			//noop
		}

		SIMDDoubleGroup(
				const std::vector<double> &doc1,
				const std::vector<double> &doc2,
				const std::vector<double> &doc3,
				const std::vector<double> &doc4
		) {
			const ulong size = doc1.size();

			assert(size == doc2.size());
			assert(doc2.size() == doc3.size());
			assert(doc3.size() == doc4.size());


			for (ulong i = 0; i < size; i++) {
				this->add(doc1[i], doc2[i], doc3[i], doc4[i]);
			}
		}

		void add(double v1, double v2, double v3, double v4) {
			const double d[] = {v1, v2, v3, v4};
			this->values.push_back(_mm256_load_pd(d));
		}

		void addFourTimes(double v) {
			this->add(v, v, v, v);
		}

		[[nodiscard]] static std::vector<SIMDDoubleGroup> groupByFour(const std::vector<std::vector<double>> &values) {
			ulong size = values.size();
			std::vector<SIMDDoubleGroup> ret;
			ulong i;
			for (i = 0; i < size / 4; i++) {
				ret.emplace_back(values[i * 4], values[i * 4 + 1], values[i * 4 + 2], values[i * 4 + 3]);
			}

			if (i * 4 < size) {
				std::vector<std::vector<double>> lasts;
				for (i = i * 4; i < size; i++) {
					lasts.push_back(values[i]);
				}

				while (lasts.size() < 4) {
					lasts.push_back(values[size - 1]);
				}

				ret.emplace_back(lasts[0], lasts[1], lasts[2], lasts[3]);
			}

			return ret;
		}
};


#endif //FOREST_TREE_EVALUATOR_SIMDDOUBLEGROUP_H
