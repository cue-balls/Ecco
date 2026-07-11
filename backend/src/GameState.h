//GameState helper file

#pragma once
#include <iostream>
#include <vector>
#include <cstdint>

extern std::vector<std::string> de;

struct UndoState {
    std::uint8_t captured_piece;
    std::uint8_t en_passant_square;
    std::uint8_t castling_rights;
};

struct check_t {
    std::uint8_t piece;
    std::uint8_t square;
};

class GameState {

    bool operator==(const GameState& other) const {
        bool out = true;
        for (int i = 0; i < 14; i++) {
            if (other.bitboards[i] != bitboards[i]) {
                return false;
            }
        }

        if (other.white_to_move != white_to_move) {
            return false;
        }

        if (other.state_stack[other.state_stack.size() - 1].castling_rights != state_stack[state_stack.size() - 1].castling_rights) {
            return false;
        }

        if (other.state_stack[other.state_stack.size() - 1].en_passant_square != state_stack[state_stack.size() - 1].en_passant_square) {
            return false;
        }

        return true;
    }

    public:
        std::vector<std::uint64_t> bitboards;
        std::vector<UndoState> state_stack;
        std::vector<std::uint8_t> mailbox;
        std::uint64_t hash_key;
        bool white_to_move;

        //piece lookup array
        inline static const char piece_order[] = {'P', 'N', 'B', 'R', 'Q', 'K', '-', 'p', 'n', 'b', 'r', 'q', 'k', '-'};
        inline static const int piece_value[] = {100, 300, 300, 500, 900, 0, 0, 100, 300, 300, 500, 900, 0, 0};
        inline static std::vector<std::vector<std::uint64_t>> piece_attacks;
        
        GameState(std::string FEN);
        void view_gamestate();


        static void populate_attacks();
        bool in_check(bool white);
        bool validate_move(std::uint16_t move);
        bool kingside_eligibility();
        bool queenside_eligibility();

        void make_move(std::uint16_t move);
        void unmake_move(std::uint16_t move);
        std::vector<std::uint16_t> get_legal_moves();
        int evaluate();

    private:
    

};