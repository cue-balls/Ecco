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

    private:

};