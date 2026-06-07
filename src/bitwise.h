#pragma once
#include <cstdint>
#include "GameState.h"

void set(std::uint64_t* board, int position);
void clear(std::uint64_t* board, int position);
int read(std::uint64_t board, int position);
void view_bitboard(std::uint64_t board);