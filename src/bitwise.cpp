#include <cstdint>
#include <iostream>
#include "bitwise.h"
#include "GameState.h"

void set(std::uint64_t* board, int position) {
    std::uint64_t mask = std::uint64_t(1) << position;
    *board |= mask;
}

void clear(std::uint64_t* board, int position) {
    std::uint64_t mask = ~(std::uint64_t(1) << position);
    *board &= mask;
}

int read(std::uint64_t board, int position) {
    board = board >> position;
    return board & std::uint64_t(1);
}

void view_bitboard(std::uint64_t board) {
    for (int i = 0; i < 64; i++)
    {
        std::cout << (board & 1)  << '|';
        board = board >> 1;

        if ((i + 1) % 8 == 0) {
            std::cout << std::endl;
        }
    }   
}

