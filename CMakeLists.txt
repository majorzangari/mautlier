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
    search/transposition_table.c
    search/move_ordering.c

    utils/diagnostic_tools.c
    utils/debug_printer.c
    utils/misc.c

    hash/hash.c
    hash/polyglot.c

    interface/cli.c
    interface/uci.c

    benchmark/benchmark.c

    test/perft.c

    tuning/tuning.c
    tuning/data_parse.c
)

target_include_directories(mautlier PRIVATE
    ${CMAKE_SOURCE_DIR}/state
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/hash
    ${CMAKE_SOURCE_DIR}/evaluation
    ${CMAKE_SOURCE_DIR}/search
    ${CMAKE_SOURCE_DIR}/interface
    ${CMAKE_SOURCE_DIR}/test
    ${CMAKE_SOURCE_DIR}/benchmark
    ${CMAKE_SOURCE_DIR}/tuning
)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(mautlier PRIVATE ${MATH_LIBRARY})
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(mautlier PRIVATE MAUTLIER_DEBUG)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "BallerRelease")
    message(STATUS "Configuring mautlier for MAXIMUM SPEED (no regard for size or compile time)")

    target_compile_options(mautlier PRIVATE
        -O3
        -Ofast
        -march=native
        -mtune=native
        -flto
        -fomit-frame-pointer
        -funroll-loops
        -finline-functions
        -fprefetch-loop-arrays
        -fstrict-aliasing
        -ffast-math
        -ftree-vectorize
        -DNDEBUG
    )

    if(NOT CMAKE_C_COMPILER_TARGET)
        include(CheckIPOSupported)
        check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
        if(ipo_supported)
            set_property(TARGET mautlier PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        else()
            message(WARNING "IPO/LTO not supported: ${ipo_error}")
        endif()
    else()
        message(STATUS "Skipping IPO/LTO check for cross-compiling")
    endif()

    target_link_options(mautlier PRIVATE
      -flto
      -Wl,-O3
    )
endif()

file(COPY ${CMAKE_SOURCE_DIR}/book DESTINATION ${CMAKE_BINARY_DIR})
