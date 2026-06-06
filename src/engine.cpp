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
    
    std::cout << num << std::endl;


    std::uint64_t board = 0;
    for (int i = 0; i < 16; i++) {
        set(&board, i);
    }

    view_bitboard(board);


    return 0;
}