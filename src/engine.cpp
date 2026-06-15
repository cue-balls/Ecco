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
    
    GameState state("5k2/8/1q6/8/1Q6/3R4/5B2/1K6 w - - 0 1");

    //state.make_move(2745);
    //state.make_move(1153);
    //state.make_move(1642);
    //state.make_move(2130);
    //state.make_move(17049);
    //state.make_move(19617);
    

    state.view_gamestate();
    //std::cout << std::endl;
    //view_bitboard(state.bitboards[6]);
    //std:: cout << std::endl;
    //view_bitboard(state.bitboards[13]);

    //std::cout << state.white_to_move << std::endl;
    //std::cout << (int)(state.state_stack[state.state_stack.size() - 1].castling_rights) << std::endl;
    //std::cout << (int)(state.state_stack[state.state_stack.size() - 1].en_passant_square) << std::endl;
    //std::cout << (state.state_stack[state.state_stack.size() - 1].captured_piece) << std::endl;
    //std::cout << state.square_occupancy(4) << std::endl;
    //std::cout << state.in_check(true) << std::endl;
    //std::cout << state.in_check(false) << std::endl;
    std::vector<std::uint8_t> rook_moves = state.get_attack_ray('Q', 27);

    for (int i = 0; i < rook_moves.size(); i++)
    {
        //std::cout << (int)rook_moves[i] << std::endl;
    }

    //state.white_to_move = !state.white_to_move;
    std::vector<std::uint16_t> moves = state.get_legal_moves();
    std::cout << moves.size() << std::endl;

    for (std::uint16_t move : moves)
    {
        std::string from = int_to_square(move & 63);
        std::string to = int_to_square((move >> 6) & 63);
        std::cout << from << to << std::endl;
    }


    return 0;
}