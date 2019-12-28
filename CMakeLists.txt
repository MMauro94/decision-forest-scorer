cmake_minimum_required(VERSION 3.10)
project(forest_tree_evaluator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O3 -mavx512vl -mavx512cd -mavx512f")

file(GLOB json_files
        "src/rapidjson/*.h"
        "src/rapidjson/*/*.h"
        )

add_executable(forest_tree_evaluator
        src/Tree.h
        src/Epitome.h
        src/ResultMask.h
        src/Config.h
        src/EqNode.h
        src/rapidscorer/RapidScorers.h
        src/rapidscorer/EqNodesRapidScorer.h
        src/rapidscorer/LinearizedRapidScorer.h
        src/rapidscorer/SIMDRapidScorer.h
        src/main.cpp
        #tests/epitome_test.cpp
        ${json_files}
        src/SIMDDoubleGroup.h
        src/SIMDResultMask.h
        src/DocGroup.h
        src/TestCase.h
        src/SIMDInfo.h
        src/SIMDEpitome.h
        )


find_package(OpenMP)
if (OpenMP_CXX_FOUND)
    target_link_libraries(forest_tree_evaluator PUBLIC OpenMP::OpenMP_CXX)
endif ()
