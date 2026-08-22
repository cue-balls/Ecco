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


class GameState {
    
    public:
        std::vector<std::uint64_t> bitboards;
        std::vector<UndoState> state_stack;
        std::vector<std::uint8_t> mailbox;
        std::uint64_t hash_key;
        std::int16_t white_opening_PST;
        std::int16_t white_mg_PST;
        std::int16_t white_eg_PST;
        std::int16_t black_opening_PST;
        std::int16_t black_mg_PST;
        std::int16_t black_eg_PST;
        bool white_to_move;

        //piece lookup array
        inline static const char piece_order[] = {'P', 'N', 'B', 'R', 'Q', 'K', '-', 'p', 'n', 'b', 'r', 'q', 'k', '-'};
        inline static const int piece_value[] = {100, 300, 320, 500, 900, 30000, 0, 100, 300, 320, 500, 900, 30000, 0};
        inline static std::uint64_t piece_attacks[5][64];
        inline static std::vector<std::uint64_t> zobrist_keys;
        
        GameState(std::string FEN);
        void view_gamestate();


        static void populate_attacks();
        static void generate_hash_keys();
        
        bool in_check(bool white);
        bool validate_move(std::uint16_t move);
        bool kingside_eligibility();
        bool queenside_eligibility();

        void make_move(std::uint16_t move);
        void unmake_move(std::uint16_t move);
        std::vector<std::uint16_t> get_legal_moves();
        std::uint64_t get_hash_preview(std::uint16_t move);
        
        std::int16_t evaluate();
        std::int16_t see(std::uint8_t square, bool side);
        std::uint8_t get_smallest_attacker(std::uint8_t square, bool side);

    private:
    

};