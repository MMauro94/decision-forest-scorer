//
// Created by molin on 28/12/2019.
//

#ifndef FOREST_TREE_EVALUATOR_SIMDINFO_H
#define FOREST_TREE_EVALUATOR_SIMDINFO_H

#include <immintrin.h>

class SIMD512InfoX8 {
	public:
		typedef uint64_t base_type;
		typedef __m512i type;
		typedef __mmask8 mask_type;
		constexpr unsigned static int bits = sizeof(base_type) * 8;
		constexpr unsigned static int groups = sizeof(type) / sizeof(base_type);


		static type set1(base_type a) {
			return _mm512_set1_epi64(a);
		}

		static type setZero() {
			return _mm512_setzero_si512();//Even though it's not 64, that's fine
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

		static type mask_set1(type source, mask_type mask, base_type val) {
			return _mm512_mask_set1_epi64(source, mask, val);
		}

		static type mask_mov(type source, mask_type mask, type val) {
			return _mm512_mask_mov_epi64(source, mask, val);
		}

		static mask_type mask_eq(mask_type mask, type a, type b) {
			return _mm512_mask_cmpeq_epi64_mask(mask, a, b);
		}

		static type mask_add(type source, mask_type mask, type a, type b) {
			return _mm512_mask_add_epi64(source, mask, a, b);
		}

		static type mask_and(type source, mask_type mask, type a, type b) {
			return _mm512_mask_and_epi64(source, mask, a, b);
		}
};


class SIMD512InfoX16 {
	public:
		typedef uint32_t base_type;
		typedef __m512i type;
		typedef __mmask16 mask_type;
		constexpr unsigned static int bits = sizeof(base_type) * 8;
		constexpr unsigned static int groups = sizeof(type) / sizeof(base_type);


		static type set1(base_type a) {
			return _mm512_set1_epi32(a);
		}

		static type setZero() {
			return _mm512_setzero_si512();
		}

		static void store(void *address, type val) {
			_mm512_mask_storeu_epi32(address, -1, val);
		}

		static type lzcnt(type val) {
			return _mm512_lzcnt_epi32(val);
		}

		static type add(type a, type b) {
			return _mm512_add_epi32(a, b);
		}

		static type mask_set1(type source, mask_type mask, base_type val) {
			return _mm512_mask_set1_epi32(source, mask, val);
		}

		static type mask_mov(type source, mask_type mask, type val) {
			return _mm512_mask_mov_epi32(source, mask, val);
		}

		static mask_type mask_eq(mask_type mask, type a, type b) {
			return _mm512_mask_cmpeq_epi32_mask(mask, a, b);
		}

		static type mask_add(type source, mask_type mask, type a, type b) {
			return _mm512_mask_add_epi32(source, mask, a, b);
		}

		static type mask_and(type source, mask_type mask, type a, type b) {
			return _mm512_mask_and_epi32(source, mask, a, b);
		}
};

class SIMD512InfoX32 {
	public:
		typedef uint16_t base_type;
		typedef __m512i type;
		typedef __mmask32 mask_type;
		constexpr unsigned static int bits = sizeof(base_type) * 8;
		constexpr unsigned static int groups = sizeof(type) / sizeof(base_type);


		static type set1(base_type a) {
			return _mm512_set1_epi16(a);
		}

		static type setZero() {
			return _mm512_setzero_si512();
		}

		static void store(void *address, type val) {
			_mm512_mask_storeu_epi16(address, -1, val);
		}

		static type lzcnt(type val) {
			//_mm512_lzcnt_epi16 doesn't exists
			auto evenRet = _mm512_rol_epi32(_mm512_lzcnt_epi32(val), 16);
			auto oddRet = _mm512_lzcnt_epi32(_mm512_rol_epi32(val, 16));
			return _mm512_or_si512(evenRet, oddRet);
		}

		static type add(type a, type b) {
			return _mm512_add_epi16(a, b);
		}

		static type mask_set1(type source, mask_type mask, base_type val) {
			return _mm512_mask_set1_epi16(source, mask, val);
		}

		static type mask_mov(type source, mask_type mask, type val) {
			return _mm512_mask_mov_epi16(source, mask, val);
		}

		static mask_type mask_eq(mask_type mask, type a, type b) {
			return _mm512_mask_cmpeq_epi16_mask(mask, a, b);
		}

		static type mask_add(type source, mask_type mask, type a, type b) {
			return _mm512_mask_add_epi16(source, mask, a, b);
		}

		static type mask_and(type source, mask_type mask, type a, type b) {
			//_mm512_mask_and_epi16 doesn't exists
			return _mm512_mask_mov_epi16(source, mask, _mm512_and_si512(a, b));
		}
};


class SIMD512InfoX64 {
	public:
		typedef uint8_t base_type;
		typedef __m512i type;
		typedef __mmask64 mask_type;
		constexpr unsigned static int bits = sizeof(base_type) * 8;
		constexpr unsigned static int groups = sizeof(type) / sizeof(base_type);


		static type set1(base_type a) {
			return _mm512_set1_epi8(a);
		}

		static type setZero() {
			return _mm512_setzero_si512();
		}

		static void store(void *address, type val) {
			_mm512_mask_storeu_epi8(address, -1, val);
		}

		static type lzcnt(type val) {
			//_mm512_lzcnt_epi8 doesn't exists
			auto oneRet = _mm512_rol_epi32(_mm512_lzcnt_epi32(val), 24);
			auto twoRet = _mm512_rol_epi32(_mm512_lzcnt_epi32(_mm512_rol_epi32(val, 8)), 16);
			auto threeRet = _mm512_rol_epi32(_mm512_lzcnt_epi32(_mm512_rol_epi32(val, 16)), 8);
			auto fourRet = _mm512_lzcnt_epi32(_mm512_rol_epi32(val, 24));
			return _mm512_or_si512(_mm512_or_si512(oneRet, twoRet), _mm512_or_si512(threeRet, fourRet));
		}

		static type add(type a, type b) {
			return _mm512_add_epi8(a, b);
		}

		static type mask_set1(type source, mask_type mask, base_type val) {
			return _mm512_mask_set1_epi8(source, mask, val);
		}

		static type mask_mov(type source, mask_type mask, type val) {
			return _mm512_mask_mov_epi8(source, mask, val);
		}

		static mask_type mask_eq(mask_type mask, type a, type b) {
			return _mm512_mask_cmpeq_epi8_mask(mask, a, b);
		}

		static type mask_add(type source, mask_type mask, type a, type b) {
			return _mm512_mask_add_epi8(source, mask, a, b);
		}

		static type mask_and(type source, mask_type mask, type a, type b) {
			//_mm512_mask_and_epi8 doesn't exists
			return _mm512_mask_mov_epi8(source, mask, _mm512_and_si512(a, b));
		}
};


class SIMD256InfoX8 {
	public:
		typedef uint32_t base_type;
		typedef __m256i type;
		typedef __mmask8 mask_type;
		constexpr unsigned static int bits = sizeof(base_type) * 8;
		constexpr unsigned static int groups = sizeof(type) / sizeof(base_type);


		static type set1(base_type a) {
			return _mm256_set1_epi32(a);
		}

		static type setZero() {
			return _mm256_setzero_si256();
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

		static type mask_set1(type source, mask_type mask, base_type val) {
			return _mm256_mask_set1_epi32(source, mask, val);
		}

		static type mask_mov(type source, mask_type mask, type val) {
			return _mm256_mask_mov_epi32(source, mask, val);
		}

		static mask_type mask_eq(mask_type mask, type a, type b) {
			return _mm256_mask_cmpeq_epi32_mask(mask, a, b);
		}

		static type mask_add(type source, mask_type mask, type a, type b) {
			return _mm256_mask_add_epi32(source, mask, a, b);
		}

		static type mask_and(type source, mask_type mask, type a, type b) {
			return _mm256_mask_and_epi32(source, mask, a, b);
		}
};
class SIMD256InfoX16 {
	public:
		typedef uint16_t base_type;
		typedef __m256i type;
		typedef __mmask32 mask_type;
		constexpr unsigned static int bits = sizeof(base_type) * 8;
		constexpr unsigned static int groups = sizeof(type) / sizeof(base_type);


		static type set1(base_type a) {
			return _mm256_set1_epi16(a);
		}

		static type setZero() {
			return _mm256_setzero_si256();
		}

		static void store(void *address, type val) {
			_mm256_mask_storeu_epi16(address, -1, val);
		}

		static type lzcnt(type val) {
			//_mm256_lzcnt_epi16 doesn't exists
			auto evenRet = _mm256_rol_epi32(_mm256_lzcnt_epi32(val), 16);
			auto oddRet = _mm256_lzcnt_epi32(_mm256_rol_epi32(val, 16));
			return _mm256_or_si256(evenRet, oddRet);
		}

		static type add(type a, type b) {
			return _mm256_add_epi16(a, b);
		}

		static type mask_set1(type source, mask_type mask, base_type val) {
			return _mm256_mask_set1_epi16(source, mask, val);
		}

		static type mask_mov(type source, mask_type mask, type val) {
			return _mm256_mask_mov_epi16(source, mask, val);
		}

		static mask_type mask_eq(mask_type mask, type a, type b) {
			return _mm256_mask_cmpeq_epi16_mask(mask, a, b);
		}

		static type mask_add(type source, mask_type mask, type a, type b) {
			return _mm256_mask_add_epi16(source, mask, a, b);
		}

		static type mask_and(type source, mask_type mask, type a, type b) {
			//_mm256_mask_and_epi16 doesn't exists
			return _mm256_mask_mov_epi16(source, mask, _mm256_and_si256(a, b));
		}
};

class SIMD256InfoX32 {
	public:
		typedef uint8_t base_type;
		typedef __m256i type;
		typedef __mmask32 mask_type;
		constexpr unsigned static int bits = sizeof(base_type) * 8;
		constexpr unsigned static int groups = sizeof(type) / sizeof(base_type);


		static type set1(base_type a) {
			return _mm256_set1_epi8(a);
		}

		static type setZero() {
			return _mm256_setzero_si256();
		}

		static void store(void *address, type val) {
			_mm256_mask_storeu_epi8(address, -1, val);
		}

		static type lzcnt(type val) {
			//_mm256_lzcnt_epi8 doesn't exists
			auto oneRet = _mm256_rol_epi32(_mm256_lzcnt_epi32(val), 24);
			auto twoRet = _mm256_rol_epi32(_mm256_lzcnt_epi32(_mm256_rol_epi32(val, 8)), 16);
			auto threeRet = _mm256_rol_epi32(_mm256_lzcnt_epi32(_mm256_rol_epi32(val, 16)), 8);
			auto fourRet = _mm256_lzcnt_epi32(_mm256_rol_epi32(val, 24));
			return _mm256_or_si256(_mm256_or_si256(oneRet, twoRet), _mm256_or_si256(threeRet, fourRet));
		}

		static type add(type a, type b) {
			return _mm256_add_epi8(a, b);
		}

		static type mask_set1(type source, mask_type mask, base_type val) {
			return _mm256_mask_set1_epi8(source, mask, val);
		}

		static type mask_mov(type source, mask_type mask, type val) {
			return _mm256_mask_mov_epi8(source, mask, val);
		}

		static mask_type mask_eq(mask_type mask, type a, type b) {
			return _mm256_mask_cmpeq_epi8_mask(mask, a, b);
		}

		static type mask_add(type source, mask_type mask, type a, type b) {
			return _mm256_mask_add_epi8(source, mask, a, b);
		}

		static type mask_and(type source, mask_type mask, type a, type b) {
			//_mm256_mask_and_epi8 doesn't exists
			return _mm256_mask_mov_epi8(source, mask, _mm256_and_si256(a, b));
		}
};

class SIMD128InfoX8 {
	public:
		typedef uint16_t base_type;
		typedef __m128i type;
		typedef __mmask8 mask_type;
		constexpr unsigned static int bits = sizeof(base_type) * 8;
		constexpr unsigned static int groups = sizeof(type) / sizeof(base_type);


		static type set1(base_type a) {
			return _mm_set1_epi16(a);
		}

		static type setZero() {
			return _mm_setzero_si128();
		}

		static void store(void *address, type val) {
			_mm_mask_storeu_epi16(address, -1, val);
		}

		static type lzcnt(type val) {
			//_mm_lzcnt_epi16 doesn't exists
			auto evenRet = _mm_rol_epi32(_mm_lzcnt_epi32(val), 16);
			auto oddRet = _mm_lzcnt_epi32(_mm_rol_epi32(val, 16));
			return _mm_or_si128(evenRet, oddRet);
		}

		static type add(type a, type b) {
			return _mm_add_epi16(a, b);
		}

		static type mask_set1(type source, mask_type mask, base_type val) {
			return _mm_mask_set1_epi16(source, mask, val);
		}

		static type mask_mov(type source, mask_type mask, type val) {
			return _mm_mask_mov_epi16(source, mask, val);
		}

		static mask_type mask_eq(mask_type mask, type a, type b) {
			return _mm_mask_cmpeq_epi16_mask(mask, a, b);
		}

		static type mask_add(type source, mask_type mask, type a, type b) {
			return _mm_mask_add_epi16(source, mask, a, b);
		}

		static type mask_and(type source, mask_type mask, type a, type b) {
			//_mm_mask_and_epi16 doesn't exists
			return _mm_mask_mov_epi16(source, mask, _mm_and_si128(a, b));
		}
};

class SIMD128InfoX16 {
	public:
		typedef uint8_t base_type;
		typedef __m128i type;
		typedef __mmask16 mask_type;
		constexpr unsigned static int bits = sizeof(base_type) * 8;
		constexpr unsigned static int groups = sizeof(type) / sizeof(base_type);


		static type set1(base_type a) {
			return _mm_set1_epi8(a);
		}

		static type setZero() {
			return _mm_setzero_si128();
		}

		static void store(void *address, type val) {
			_mm_mask_storeu_epi8(address, -1, val);
		}

		static type lzcnt(type val) {
			//_mm_lzcnt_epi8 doesn't exists
			auto oneRet = _mm_rol_epi32(_mm_lzcnt_epi32(val), 24);
			auto twoRet = _mm_rol_epi32(_mm_lzcnt_epi32(_mm_rol_epi32(val, 8)), 16);
			auto threeRet = _mm_rol_epi32(_mm_lzcnt_epi32(_mm_rol_epi32(val, 16)), 8);
			auto fourRet = _mm_lzcnt_epi32(_mm_rol_epi32(val, 24));
			return _mm_or_si128(_mm_or_si128(oneRet, twoRet), _mm_or_si128(threeRet, fourRet));
		}

		static type add(type a, type b) {
			return _mm_add_epi8(a, b);
		}

		static type mask_set1(type source, mask_type mask, base_type val) {
			return _mm_mask_set1_epi8(source, mask, val);
		}

		static type mask_mov(type source, mask_type mask, type val) {
			return _mm_mask_mov_epi8(source, mask, val);
		}

		static mask_type mask_eq(mask_type mask, type a, type b) {
			return _mm_mask_cmpeq_epi8_mask(mask, a, b);
		}

		static type mask_add(type source, mask_type mask, type a, type b) {
			return _mm_mask_add_epi8(source, mask, a, b);
		}

		static type mask_and(type source, mask_type mask, type a, type b) {
			//_mm_mask_and_epi8 doesn't exists
			return _mm_mask_mov_epi8(source, mask, _mm_and_si128(a, b));
		}
};

#endif //FOREST_TREE_EVALUATOR_SIMDINFO_H
