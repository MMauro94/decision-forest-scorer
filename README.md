# forest-tree-evaluator

##initial implementation
###parallelizing the forest
Single: 7,008,971 ns

###parallelizing feature
Single: 92,123,575 ns

###parallelizing documents
Single: 6,364,3‬00 ns
Single -O3: 1,573,000 ns

## linearized eqnodes
Single -O3: 1,565,840 ns

### binary search
Single -O3: 1,431,740 ns

### binary search + linearized mask
Single -O3:

// TODO: misuarare sbilanciamento feature
#VM
4 core

#SIMD 512

| Optimizations | Time/document |
|---|---|
| SIMD 512 | 4,691,575 ns |
| SIMD 512 + Parallel documents (2 threads) | 2,444,666 ns |
| SIMD 512 + Parallel documents (3 threads) | 2,591,520 ns |
| SIMD 512 + Parallel documents (4 threads) | 2,671,842 ns |
| SIMD 512 + Parallel documents (8 threads) | 2,929,893 ns |
| SIMD 512 + Parallel forests (2 threads) | 2,382,183 ns |
| SIMD 512 + Parallel forests (3 threads) | 3,022,223 ns |
| SIMD 512 + Parallel forests (4 threads) | 2,310,563 ns |
| SIMD 512 + Parallel forests (8 threads) | **2,194,436** ns |
| SIMD 512 + Parallel forests (16 threads) | **2,051,346** ns |

#SIMD 256
| Optimizations | Time/document |
|---|---|
| SIMD 256 | 4,691,575 ns |
| SIMD 256 + Parallel documents (2 threads) |  ns |
| SIMD 256 + Parallel documents (3 threads) |  ns |
| SIMD 256 + Parallel documents (4 threads) |  ns |
| SIMD 256 + Parallel documents (8 threads) |  ns |
| SIMD 256 + Parallel forests (2 threads) |  ns |
| SIMD 256 + Parallel forests (3 threads) |  ns |
| SIMD 256 + Parallel forests (4 threads) |  ns |
| SIMD 256 + Parallel forests (8 threads) |  ns |