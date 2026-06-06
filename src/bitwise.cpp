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

void view_gamestate(GameState state) {
    std::vector<char> board_representation(64);
    
    for (int i = 0; i < 64; i++) {
        board_representation[i] = '-';
    }

    for (int i = 0; i < 14; i++)
    {
        if (i == 6 || i == 13) {
            continue;
        }

        std::uint64_t board = state.bitboards[i];
        for (int j = 0; j < 64; j++)
        {
            if (read(board, j)) {
                switch (i) {
                    case 0: board_representation[j] = 'P'; break;
                    case 1: board_representation[j] = 'N'; break;
                    case 2: board_representation[j] = 'B'; break;
                    case 3: board_representation[j] = 'R'; break;
                    case 4: board_representation[j] = 'Q'; break;
                    case 5: board_representation[j] = 'K'; break;
                    case 7: board_representation[j] = 'p'; break;
                    case 8: board_representation[j] = 'n'; break;
                    case 9: board_representation[j] = 'b'; break;
                    case 10: board_representation[j] = 'r'; break;
                    case 11: board_representation[j] = 'q'; break;
                    case 12: board_representation[j] = 'k'; break;
                }
            }
        }
    }

    for (int i = 0; i < 64; i++)
    {
        std::cout << board_representation[i] << '|';
        if ((i + 1) % 8 == 0)
        {
            std::cout << std::endl;
        }
    }
}