//
// Created by molin on 10/12/2019.
//

#ifndef FOREST_TREE_EVALUATOR_SIMDDOUBLEGROUP_H
#define FOREST_TREE_EVALUATOR_SIMDDOUBLEGROUP_H

#include <immintrin.h>
#include <cassert>
#include <vector>

class SIMDDoubleGroup {
		std::vector<__m512d> values;

	public:
		SIMDDoubleGroup() {
			//noop
		}

		SIMDDoubleGroup(
				const std::vector<double> &doc1,
				const std::vector<double> &doc2,
				const std::vector<double> &doc3,
				const std::vector<double> &doc4,
				const std::vector<double> &doc5,
				const std::vector<double> &doc6,
				const std::vector<double> &doc7,
				const std::vector<double> &doc8
		) {
			const ulong size = doc1.size();

			assert(size == doc2.size());
			assert(doc2.size() == doc3.size());
			assert(doc3.size() == doc4.size());
			assert(doc4.size() == doc5.size());
			assert(doc5.size() == doc6.size());
			assert(doc6.size() == doc7.size());
			assert(doc7.size() == doc8.size());


			for (ulong i = 0; i < size; i++) {
				this->add(doc1[i], doc2[i], doc3[i], doc4[i], doc5[i], doc6[i], doc7[i], doc8[i]);
			}
		}

		void add(double v1, double v2, double v3, double v4, double v5, double v6, double v7, double v8) {
			const double d[] = {v1, v2, v3, v4, v5, v6, v7, v8};
			this->values.push_back(_mm512_load_pd(d));
		}

		void addEightTimes(double v) {
			this->add(v, v, v, v, v, v, v, v);
		}

		[[nodiscard]] __m512d get(unsigned long index) const {
			return this->values[index];
		}

		[[nodiscard]] unsigned long size() const {
			return this->values.size();
		}

		[[nodiscard]] static std::vector<SIMDDoubleGroup>
		groupByEight(const std::vector<std::vector<double>> &values) {
			ulong size = values.size();
			std::vector<SIMDDoubleGroup> ret;
			ulong i;
			for (i = 0; i < size / 8; i++) {
				ret.emplace_back(
						values[i * 8],
						values[i * 8 + 1],
						values[i * 8 + 2],
						values[i * 8 + 3],
						values[i * 8 + 4],
						values[i * 8 + 5],
						values[i * 8 + 6],
						values[i * 8 + 7]
				);
			}

			if (i * 8 < size) {
				std::vector<std::vector<double>> lasts;
				for (i = i * 8; i < size; i++) {
					lasts.push_back(values[i]);
				}

				while (lasts.size() < 8) {
					lasts.push_back(values[size - 1]);
				}

				ret.emplace_back(lasts[0], lasts[1], lasts[2], lasts[3], lasts[4], lasts[5], lasts[6], lasts[7]);
			}

			return ret;
		}
};


#endif //FOREST_TREE_EVALUATOR_SIMDDOUBLEGROUP_H
