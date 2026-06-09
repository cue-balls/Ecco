//GameState helper file

#pragma once
#include <iostream>
#include <vector>
#include <cstdint>

struct UndoState {
    char captured_piece;
    std::uint8_t en_passant_square;
    std::uint8_t castling_rights;
};

class GameState {
    public:
        std::vector<std::uint64_t> bitboards;
        std::vector<UndoState> state_stack;
        bool white_to_move;

        inline static char piece_order[] = {'P', 'N', 'B', 'R', 'Q', 'K', '-', 'p', 'n', 'b', 'r', 'q', 'k'};
        
        GameState();
        GameState(std::string FEN);
        void view_gamestate();
        char square_occupancy(std::uint8_t square);

    private:

};