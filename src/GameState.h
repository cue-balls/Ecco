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
        
        GameState();
        GameState(std::string FEN);
        void view_gamestate();
        static std::uint8_t square_to_int(std::string square);
        static std::string int_to_square(std::uint8_t square);

    private:

};