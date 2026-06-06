#include <iostream>
#include <cstdint>
#include "GameState.h"
#include "bitwise.h"


int main() {
    //GameState::greet();
    std::uint64_t num = 0;
    set(&num, 4);
    set(&num, 1);
    clear(&num, 4);
    
    std::cout << read(num, 1) << std::endl;


    std::uint64_t board = 0;
    for (int i = 32; i < 56; i++) {
        set(&board, i);
    }

    //view_bitboard(board);
    GameState state;
    view_gamestate(state);

    for (int i = 0; i < 14; i++)
    {
        //view_bitboard(state.bitboards[i]);
        //std::cout << std::endl;
    }


    return 0;
}