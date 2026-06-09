//bitwise helper file

#pragma once
#include <cstdint>
#include "GameState.h"

void set(std::uint64_t* board, int position);
void set(std::uint8_t* board, int position);
void clear(std::uint64_t* board, int position);
void clear(std::uint8_t* board, int position);
int read(std::uint64_t board, int position);
void view_bitboard(std::uint64_t board);
std::uint8_t square_to_int(std::string square);
std::string int_to_square(std::uint8_t square);