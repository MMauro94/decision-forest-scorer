//
// Created by MMarco on 05/11/2019.
//
#ifndef CONFIG
#define CONFIG
/*
class Config {
 bool parallel_mask;
 bool parallel_score;
 bool parallel_forests;
 bool parallel_documents;
 int number_of_threads;
 int max_documents;


};
*/
#define PARALLEL_MASK false
#define PARALLEL_SCORE false
#define PARALLEL_FORESTS true
#define PARALLEL_DOCUMENTS false

#define NUMBER_OF_THREADS 16

#include <immintrin.h>

typedef std::uint16_t BLOCK;
constexpr auto MASK_SIZE = sizeof(BLOCK) * 8;

#if PARALLEL_MASK
typedef std::atomic_uint16_t MaskType;//TODO: provare ad usare istruzione al posto di atomic
#else
typedef BLOCK MaskType;
#endif

#endif