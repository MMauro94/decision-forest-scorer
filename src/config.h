//
// Created by MMarco on 05/11/2019.
//
#ifndef CONFIG
#define CONFIG

#define PARALLEL_MASK false
#define PARALLEL_SCORE false
#define PARALLEL_FORESTS false
#define PARALLEL_DOCUMENTS true
#define RAPID_SCORER_IMPL SIMDRapidScorer
#define RAPID_SCORER_IMPL_STR "SIMDRapidScorer.h"

typedef std::uint16_t BLOCK;
constexpr auto MASK_SIZE = sizeof(BLOCK) * 8;

#if PARALLEL_MASK
typedef std::atomic_uint16_t MaskType;//TODO: provare ad usare istruzione al posto di atomic
#else
typedef BLOCK MaskType;
#endif

#endif