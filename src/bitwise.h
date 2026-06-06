#pragma once
#include <cstdint>

void set(std::uint64_t* board, int position);
void clear(std::uint64_t* board, int position);
void view_bitboard(std::uint64_t board);