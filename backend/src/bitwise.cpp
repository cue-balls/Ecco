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

