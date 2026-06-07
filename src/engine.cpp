#include <iostream>
#include <cstdint>
#include "GameState.h"
#include "bitwise.h"


int main() {
    std::uint64_t num = 0;
    set(&num, 4);
    set(&num, 1);
    clear(&num, 4);
    
    //std::cout << read(num, 1) << std::endl;


    std::uint64_t board = 0;
    for (int i = 0; i < 16; i++) {
        set(&board, i);
    }

    //view_bitboard(board);
    //std::cout << std::endl;
    
    GameState state("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");
    state.view_gamestate();
    std::cout << state.white_to_move << std::endl;
    std::cout << GameState::square_to_int("e5") << std::endl;
    std::cout << GameState::int_to_square(63) << std::endl;


    return 0;
}