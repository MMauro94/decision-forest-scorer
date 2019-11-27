//
// Created by MMarco on 05/11/2019.
//

#define PARALLEL_MASK false
#define PARALLEL_SCORE false
#define PARALLEL_FORESTS false
#define PARALLEL_DOCUMENTS true
#define PARALLEL_MASK_INIT false
#define LINEARIZE_EQ_NODES true

#if PARALLEL_MASK
typedef std::vector<std::atomic_uint8_t> MaskType;//TODO: provare ad usare istruzione al posto di atomic
#else
typedef std::vector<std::uint8_t> MaskType;//TODO: non usare vector
#endif