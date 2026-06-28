//bitwise helper file

#pragma once
#include <cstdint>
#include <vector>
#include "GameState.h"


void set(std::uint64_t* board, int position);
void set(std::uint8_t* board, int position);
void set(std::uint16_t* board, int position);
void clear(std::uint64_t* board, int position);
void clear(std::uint8_t* board, int position);
int read(std::uint64_t board, int position);
void view_bitboard(std::uint64_t board);
int bitscan_forward(std::uint64_t bitboard);
int bitscan_reverse(std::uint64_t bitboard);
std::vector<std::uint8_t> serialize(std::uint64_t bitboard);
std::uint8_t square_to_int(std::string square);
std::string int_to_square(std::uint8_t square);

std::uint64_t fillNorth(std::uint64_t gen);
std::uint64_t fillSouth(std::uint64_t gen);
std::uint64_t fillEast(std::uint64_t gen);
std::uint64_t fillWest(std::uint64_t gen);

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
}

