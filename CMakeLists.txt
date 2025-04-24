cmake_minimum_required(VERSION 3.31)
project(mautlier C)

set(CMAKE_C_STANDARD 11)

add_executable(mautlier main.c
        state/board.h
        state/board.c
        state/fen.h
        state/fen.c
        state/move.h
        state/move.c)
