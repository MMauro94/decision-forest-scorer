cmake_minimum_required(VERSION 3.10)
project(forest_tree_evaluator)

set(CMAKE_CXX_STANDARD 17)

add_executable(forest_tree_evaluator Tree.cpp Tree.h FeaturesMap.cpp FeaturesMap.h Epitome.cpp Epitome.h tests/tree_test.cpp ResultMask.cpp ResultMask.h)


find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(forest_tree_evaluator PUBLIC OpenMP::OpenMP_CXX)
endif()
