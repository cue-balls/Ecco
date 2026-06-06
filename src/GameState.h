#pragma once
#include <iostream>
#include <vector>
#include <cstdint>

class GameState {
    public:
        std::vector<std::uint64_t> bitboards;
        bool white_to_move;
    
        static void greet();

    private:

};