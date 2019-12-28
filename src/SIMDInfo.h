//
// Created by molin on 28/12/2019.
//

#ifndef FOREST_TREE_EVALUATOR_SIMDINFO_H
#define FOREST_TREE_EVALUATOR_SIMDINFO_H

#include <immintrin.h>

class SIMD512Info {
	public:
		typedef int64_t base_type;
		typedef __m512i type;
		constexpr unsigned static int bits = sizeof(base_type) * 8;


		static type set1(base_type a) {
			return _mm512_set1_epi64(a);
		}

		static void store(void *address, type val) {
			_mm512_store_epi64(address, val);
		}

		static type lzcnt(type val) {
			return _mm512_lzcnt_epi64(val);
		}

		static type add(type a, type b) {
			return _mm512_add_epi64(a, b);
		}

		static type mask_set1(type source, __mmask8 mask, base_type val) {
			return _mm512_mask_set1_epi64(source, mask, val);
		}

		static type mask_mov(type source, __mmask8 mask, type val) {
			return _mm512_mask_mov_epi64(source, mask, val);
		}

		static __mmask8 mask_cmp_mask(__mmask8 mask, type a, type b, const int operation) {
			return _mm512_mask_cmp_epi64_mask(mask, a, b, operation);
		}

		static type mask_add(type source, __mmask8 mask, type a, type b) {
			return _mm512_mask_add_epi64(source, mask, a, b);
		}

		static type mask_and(type source, __mmask8 mask, type a, type b) {
			return _mm512_mask_and_epi64(source, mask, a, b);
		}
};


class SIMD256Info {
	public:
		typedef int32_t base_type;
		typedef __m256i type;
		constexpr unsigned static int bits = sizeof(base_type) * 8;


		static type set1(base_type a) {
			return _mm256_set1_epi32(a);
		}

		static void store(void *address, type val) {
			_mm256_mask_store_epi32(address, 0xFF, val);
		}

		static type lzcnt(type val) {
			return _mm256_lzcnt_epi32(val);
		}

		static type add(type a, type b) {
			return _mm256_add_epi32(a, b);
		}

		static type mask_set1(type source, __mmask8 mask, base_type val) {
			return _mm256_mask_set1_epi32(source, mask, val);
		}

		static type mask_mov(type source, __mmask8 mask, type val) {
			return _mm256_mask_mov_epi32(source, mask, val);
		}

		static __mmask8 mask_cmp_mask(__mmask8 mask, type a, type b, const int operation) {
			return _mm256_mask_cmp_epi32_mask(mask, a, b, operation);
		}

		static type mask_add(type source, __mmask8 mask, type a, type b) {
			return _mm256_mask_add_epi32(source, mask, a, b);
		}

		static type mask_and(type source, __mmask8 mask, type a, type b) {
			return _mm256_mask_and_epi32(source, mask, a, b);
		}
};

#endif //FOREST_TREE_EVALUATOR_SIMDINFO_H
