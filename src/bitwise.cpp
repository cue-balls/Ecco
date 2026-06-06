#include <cstdint>
#include <iostream>
#include "bitwise.h"

void set(std::uint64_t* board, int position) {
    std::uint64_t mask = 1 << position;
    *board |= mask;
}

void clear(std::uint64_t* board, int position) {
    std::uint64_t mask = ~(1 << position);
    *board &= mask;
}

void view_bitboard(std::uint64_t board) {
    for (int i = 0; i < 63; i++)
    {
        std::cout << (board & 1) << '|';
        board = board >> 1;

        if ((i - 7) % 8 == 0)
        {
            std::cout << std::endl;
        }
    }
    std::cout << (board & 1) << '|' << std::endl;
}