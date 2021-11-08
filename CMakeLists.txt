cmake_minimum_required(VERSION 3.0.0)
project(distinctive-either- VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(distinctive-either- main.c heuristics.c lower_bound.c node_pool.c print.c problem.c solution.c solve.c timer.c)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
