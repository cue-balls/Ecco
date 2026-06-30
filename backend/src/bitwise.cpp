#include <cstdint>
#include <iostream>
#include "bitwise.h"
#include "GameState.h"


//basic bitwise stuff
//only important consideration is that constants often must be cast to uint64_t


void set(std::uint64_t* board, int position) {
    std::uint64_t mask = std::uint64_t(1) << position;
    *board |= mask;
}

void set(std::uint8_t* board, int position) {
    std::uint64_t mask = std::uint8_t(1) << position;
    *board |= mask;
}

void set(std::uint16_t* board, int position) {
    std::uint64_t mask = std::uint16_t(1) << position;
    *board |= mask;
}

void clear(std::uint64_t* board, int position) {
    std::uint64_t mask = ~(std::uint64_t(1) << position);
    *board &= mask;
}

void clear(std::uint8_t* board, int position) {
    std::uint64_t mask = ~(std::uint8_t(1) << position);
    *board &= mask;
}

int read(std::uint64_t board, int position) {
    board = board >> position;
    return board & std::uint64_t(1);
}

//debugging tool
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

//basic conversion functions
std::uint8_t square_to_int(std::string square) {
    int num = (int)(square[1] - '0');
    int letter_pos = (int)(square[0] - 'a');
    return (8 * (8 - num)) + letter_pos;
}

std::string int_to_square(std::uint8_t square) {
    std::string out(2, '-');
    char letter = 'a' + (square % 8);
    char num = '0' + (8 - (square / 8));
    out[0] = letter;
    out[1] = num;

    return out;
}



const std::vector<int> index64 = {
    0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

int bitscan_forward(std::uint64_t bitboard) {
   const std::uint64_t debruijn64 = 0x03f79d71b4cb0a89ULL;
   return index64[((bitboard ^ (bitboard-1)) * debruijn64) >> 58];
}

int bitscan_reverse(std::uint64_t bitboard) {
   const std::uint64_t debruijn64 = 0x03f79d71b4cb0a89ULL;
   bitboard |= bitboard >> 1; 
   bitboard |= bitboard >> 2;
   bitboard |= bitboard >> 4;
   bitboard |= bitboard >> 8;
   bitboard |= bitboard >> 16;
   bitboard |= bitboard >> 32;
   return index64[(bitboard * debruijn64) >> 58];
}


std::vector<std::uint8_t> serialize(std::uint64_t bitboard) {
    std::vector<uint8_t> indices;

    while (bitboard)
    {
        std::uint8_t trailing = bitscan_forward(bitboard);
        clear(&bitboard, trailing);
        indices.push_back(trailing);
    }

    return indices;
}

std::uint64_t fill_north(std::uint64_t gen) {
    gen |= (gen >> 8);
    gen |= (gen >> 16);
    gen |= (gen >> 32);
    
    return gen;
}

std::uint64_t fill_south(std::uint64_t gen) {
    gen |= (gen << 8);
    gen |= (gen << 16);
    gen |= (gen << 32);
    
    return gen;
}

std::uint64_t fill_east(std::uint64_t gen) {
    std::uint64_t primary = ~Bitwise::AFILE;
    std::uint64_t secondary = (primary << 1) & primary;
    std::uint64_t tertiary = (secondary << 2) & secondary;
    gen |= ((gen << 1) & primary);
    gen |= ((gen << 2) & secondary);
    gen |= ((gen << 4) & tertiary);

    return gen;
}

std::uint64_t fill_west(std::uint64_t gen) {
    std::uint64_t primary = ~Bitwise::HFILE;
    std::uint64_t secondary = (primary >> 1) & primary;
    std::uint64_t tertiary = (secondary >> 2) & secondary;
    gen |= ((gen >> 1) & primary);
    gen |= ((gen >> 2) & secondary);
    gen |= ((gen >> 4) & tertiary);

    return gen;
}

std::uint64_t fill_northeast(std::uint64_t gen) {
    std::uint64_t primary = ~Bitwise::AFILE;
    std::uint64_t secondary = (primary << 1) & primary;
    std::uint64_t tertiary = (secondary << 2) & secondary;
    gen |= ((gen >> 7) & primary);
    gen |= ((gen >> 14) & secondary);
    gen |= ((gen >> 28) & tertiary);
    
    return gen;
}

std::uint64_t fill_northwest(std::uint64_t gen) {
    std::uint64_t primary = ~Bitwise::HFILE;
    std::uint64_t secondary = (primary >> 1) & primary;
    std::uint64_t tertiary = (secondary >> 2) & secondary;
    gen |= ((gen >> 9) & primary);
    gen |= ((gen >> 18) & secondary);
    gen |= ((gen >> 36) & tertiary);
    
    return gen;
}

std::uint64_t fill_southeast(std::uint64_t gen) {
    std::uint64_t primary = ~Bitwise::AFILE;
    std::uint64_t secondary = (primary << 1) & primary;
    std::uint64_t tertiary = (secondary << 2) & secondary;
    gen |= ((gen << 9) & primary);
    gen |= ((gen << 18) & secondary);
    gen |= ((gen << 36) & tertiary);
    
    return gen;
}

std::uint64_t fill_southwest(std::uint64_t gen) {
    std::uint64_t primary = ~Bitwise::HFILE;
    std::uint64_t secondary = (primary >> 1) & primary;
    std::uint64_t tertiary = (secondary >> 2) & secondary;
    gen |= ((gen << 7) & primary);
    gen |= ((gen << 14) & secondary);
    gen |= ((gen << 28) & tertiary);
    
    return gen;
}

std::uint64_t occlude_north(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square) {
    occupancy &= Bitwise::FILE_LIST[square % 8];
    occupancy &= ((~(1ULL << 63)) >> (63 - square));
    occupancy = fill_north(occupancy) >> 8;
    return ray - occupancy;
}

std::uint64_t occlude_south(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square) {
    occupancy &= Bitwise::FILE_LIST[square % 8];
    occupancy &= (~(1ULL) << (square));
    occupancy = fill_south(occupancy) << 8;
    return ray - occupancy;
}

std::uint64_t occlude_east(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square) {
    occupancy &= Bitwise::RANK_LIST[7 - square / 8];
    occupancy &= (~(1ULL) << (square));
    occupancy = fill_east(occupancy) << 1;
    return ray - (occupancy & ~Bitwise::AFILE);
}

std::uint64_t occlude_west(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square) {
    occupancy &= Bitwise::RANK_LIST[7 - square / 8];
    occupancy &= (~(1ULL << 63) >> (63 - square));
    occupancy = fill_west(occupancy) >> 1;
    return ray - (occupancy & ~Bitwise::HFILE);
}

std::uint64_t occlude_northeast(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square) {
    occupancy &= Bitwise::DIAGONALS[7 + (7 - square / 8) - square % 8];
    occupancy &= (~(1ULL << 63) >> (63 - square));
    occupancy = fill_northeast(occupancy) >> 7;
    return ray - (occupancy & ~Bitwise::AFILE);
}

std::uint64_t occlude_northwest(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square) {
    occupancy &= Bitwise::ANTI_DIAGONALS[(7 - square / 8) + square % 8];
    occupancy &= (~(1ULL << 63) >> (63 - square));
    occupancy = fill_northwest(occupancy) >> 9;
    return ray - (occupancy & ~Bitwise::HFILE);
}

std::uint64_t occlude_southeast(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square) {
    occupancy &= Bitwise::ANTI_DIAGONALS[(7 - square / 8) + square % 8];
    occupancy &= (~(1ULL) << square);
    occupancy = fill_southeast(occupancy) << 9;
    return ray - (occupancy & ~Bitwise::AFILE);
}

std::uint64_t occlude_southwest(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square) {
    occupancy &= Bitwise::DIAGONALS[7 + (7 - square / 8) - square % 8];
    occupancy &= (~(1ULL) << square);
    occupancy = fill_southwest(occupancy) << 7;
    return ray - (occupancy & ~Bitwise::HFILE);
}