cmake_minimum_required(VERSION 3.10)
project(forest_tree_evaluator)

set(CMAKE_CXX_STANDARD 17)

add_executable(forest_tree_evaluator json.hpp Tree.cpp Tree.h Epitome.cpp Epitome.h ResultMask.cpp ResultMask.h config.h EqNode.h RapidScorer.h main.cpp)


find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(forest_tree_evaluator PUBLIC OpenMP::OpenMP_CXX)
endif()
