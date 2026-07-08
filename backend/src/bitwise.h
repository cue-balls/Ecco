//bitwise helper file

#pragma once
#include <cstdint>
#include <vector>
#include "GameState.h"


std::uint64_t set(std::uint64_t board, int position);
std::uint8_t set(std::uint8_t board, int position);
std::uint16_t set(std::uint16_t board, int position);
std::uint64_t clear(std::uint64_t board, int position);
std::uint8_t clear(std::uint8_t board, int position);
std::uint8_t read(std::uint64_t board, int position);
void view_bitboard(std::uint64_t board);
std::uint8_t bitscan_forward(std::uint64_t bitboard);
std::uint8_t bitscan_reverse(std::uint64_t bitboard);
std::vector<std::uint8_t> serialize(std::uint64_t bitboard);
std::uint8_t square_to_int(std::string square);
std::string int_to_square(std::uint8_t square);

std::uint64_t fill_north(std::uint64_t gen);
std::uint64_t fill_south(std::uint64_t gen);
std::uint64_t fill_east(std::uint64_t gen);
std::uint64_t fill_west(std::uint64_t gen);
std::uint64_t fill_northeast(std::uint64_t gen);
std::uint64_t fill_northwest(std::uint64_t gen);
std::uint64_t fill_southeast(std::uint64_t gen);
std::uint64_t fill_southwest(std::uint64_t gen);

std::uint64_t occlude_north(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square);
std::uint64_t occlude_south(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square);
std::uint64_t occlude_east(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square);
std::uint64_t occlude_west(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square);
std::uint64_t occlude_northeast(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square);
std::uint64_t occlude_northwest(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square);
std::uint64_t occlude_southeast(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square);
std::uint64_t occlude_southwest(std::uint64_t ray, std::uint64_t occupancy, std::uint8_t square);

namespace Bitwise {
    inline const std::uint64_t AFILE = 0x101010101010101;
    inline const std::uint64_t BFILE = 0x202020202020202;
    inline const std::uint64_t CFILE = 0x404040404040404;
    inline const std::uint64_t DFILE = 0x808080808080808;
    inline const std::uint64_t EFILE = 0x1010101010101010;
    inline const std::uint64_t FFILE = 0x2020202020202020;
    inline const std::uint64_t GFILE = 0x4040404040404040;
    inline const std::uint64_t HFILE = 0x8080808080808080;

    inline const std::uint64_t RANK1 = 0xff00000000000000;
    inline const std::uint64_t RANK2 = 0xff000000000000;
    inline const std::uint64_t RANK3 = 0xff0000000000;
    inline const std::uint64_t RANK4 = 0xff00000000;
    inline const std::uint64_t RANK5 = 0xff000000;
    inline const std::uint64_t RANK6 = 0xff0000;
    inline const std::uint64_t RANK7 = 0xff00;
    inline const std::uint64_t RANK8 = 0xff;

    inline const std::vector<std::uint64_t> FILE_LIST = {
        AFILE,
        BFILE,
        CFILE,
        DFILE,
        EFILE,
        FFILE,
        GFILE,
        HFILE
    };

    inline const std::vector<std::uint64_t> RANK_LIST = {
        RANK1,
        RANK2,
        RANK3,
        RANK4,
        RANK5,
        RANK6,
        RANK7,
        RANK8
    };


    //rank - file
    inline const std::vector<std::uint64_t> DIAGONALS = {
        0x8000000000000000ULL,
        0x4080000000000000ULL,
        0x2040800000000000ULL,
        0x1020408000000000ULL,
        0x810204080000000ULL,
        0x408102040800000ULL,
        0x204081020408000ULL,
        0x102040810204080ULL,
        0x1020408102040ULL,
        0x10204081020ULL,
        0x102040810ULL,
        0x1020408ULL,
        0x10204ULL,
        0x102ULL,
        0x1ULL
    };

    //rank + file
    inline const std::vector<std::uint64_t> ANTI_DIAGONALS = {
        0x100000000000000ULL,
        0x201000000000000ULL,
        0x402010000000000ULL,
        0x804020100000000ULL,
        0x1008040201000000ULL,
        0x2010080402010000ULL,
        0x4020100804020100ULL,
        0x8040201008040201ULL,
        0x80402010080402ULL,
        0x804020100804ULL,
        0x8040201008ULL,
        0x80402010ULL,
        0x804020ULL,
        0x8040ULL,
        0x80ULL
    };
}

