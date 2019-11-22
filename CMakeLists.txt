cmake_minimum_required(VERSION 3.10)
project(forest_tree_evaluator)

set(CMAKE_CXX_STANDARD 17)

file(GLOB json_files
        "src/rapidjson/*.h"
        "src/rapidjson/*/*.h"
        )

add_executable(forest_tree_evaluator
        src/Tree.h
        src/Epitome.h
        src/ResultMask.h
        src/config.h
        src/EqNode.h
        src/RapidScorer.h
        src/main.cpp
        ${json_files}
        )


find_package(OpenMP)
if (OpenMP_CXX_FOUND)
    target_link_libraries(forest_tree_evaluator PUBLIC OpenMP::OpenMP_CXX)
endif ()
