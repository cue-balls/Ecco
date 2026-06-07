//GameState helper file

#pragma once
#include <iostream>
#include <vector>
#include <cstdint>

class GameState {
    public:
        std::vector<std::uint64_t> bitboards;
        bool white_to_move;
    
        GameState();
        GameState(std::string FEN);
        void view_gamestate();
        static int square_to_int(std::string square);
        static std::string int_to_square(int square);

    private:

};