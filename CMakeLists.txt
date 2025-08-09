cmake_minimum_required(VERSION 3.31)
project(mautlier C)

set(CMAKE_C_STANDARD 11)

add_executable(mautlier 
        main.c
        state/board.c
        state/fen.c
        state/move.c
        state/constants.c
        evaluation/eval.c
        search/search.c
        utils/diagnostic_tools.c
        utils/debug_printer.c
)

target_include_directories(mautlier PRIVATE
    ${CMAKE_SOURCE_DIR}/state
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/hash
    ${CMAKE_SOURCE_DIR}/evaluation
    ${CMAKE_SOURCE_DIR}/search
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(mautlier PRIVATE MAUTLIER_DEBUG)
endif()
