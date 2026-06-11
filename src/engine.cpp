//main engine logic
//frontend queries for each move after passing FEN string
//alpha_beta_search attaches a numeric value to each move
//highest value move is passed back to the frontend
//this engine takes the centipawn approach to evaluating positions to avoid floating point math

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
    
    GameState state("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1");

    state.make_move(2942);
    /*state.make_move(1153);
    state.make_move(2237);
    state.make_move(1350);
    state.make_move(12220);
    
    state.unmake_move(12220);
    state.unmake_move(1350);
    state.unmake_move(2237);
    state.unmake_move(1153);
    state.unmake_move(2942); */
    state.make_move(2267);
    state.make_move(6322);
    state.make_move(23203);
    state.make_move(6647);
    state.make_move(19562);
    state.make_move(3583);
    state.make_move(65073);

    state.unmake_move(65073);
    state.unmake_move(3583);
    state.unmake_move(19562);
    state.unmake_move(6647);
    state.unmake_move(23203);
    state.unmake_move(6322);
    state.unmake_move(2267);
    state.unmake_move(2942);

    state.view_gamestate();
    std::cout << std::endl;
    view_bitboard(state.bitboards[6]);
    std:: cout << std::endl;
    view_bitboard(state.bitboards[13]);

    std::cout << state.white_to_move << std::endl;
    //std::cout << (int)square_to_int("e5") << std::endl;
    //std::cout << int_to_square(63) << std::endl;
    std::cout << (int)(state.state_stack[state.state_stack.size() - 1].castling_rights) << std::endl;
    std::cout << (int)(state.state_stack[state.state_stack.size() - 1].en_passant_square) << std::endl;
    std::cout << (state.state_stack[state.state_stack.size() - 1].captured_piece) << std::endl;
    //std::cout << state.square_occupancy(4) << std::endl;

    return 0;
}