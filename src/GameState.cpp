//class to represent the gamestate
//this engine will use the bitboard approach with 14 bitboards to represent the gamestate
#include <iostream>
#include <vector>
#include "GameState.h"
#include "bitwise.h"
    

    void GameState::greet() {
        std::cout << "hello" << std::endl;
    }


    GameState::GameState() : bitboards(14), white_to_move(true) {
        std::uint64_t pieces = 0;
        
        for (int i = 48; i < 56; i++) {
            set(&pieces, i);
        }

        bitboards[0] = pieces;

        pieces = 0;
        set(&pieces, 56);
        set(&pieces, 63);
        bitboards[3] = pieces;

        pieces = 0;
        set(&pieces, 57);
        set(&pieces, 62);
        bitboards[1] = pieces;

        pieces = 0;
        set(&pieces, 58);
        set(&pieces, 61);
        bitboards[2] = pieces;

        pieces = 0;
        set(&pieces, 59);
        bitboards[4] = pieces;

        pieces = 0;
        set(&pieces, 60);
        bitboards[5] = pieces;

        pieces = 0;
        for (int i = 48; i < 64; i++) {
            set(&pieces, i);
        }

        bitboards[6] = pieces;

        
        pieces = 0;
        for (int i = 8; i < 16; i++) {
            set(&pieces, i);
        }

        bitboards[7] = pieces;

        pieces = 0;
        set(&pieces, 0);
        set(&pieces, 7);
        bitboards[10] = pieces;

        pieces = 0;
        set(&pieces, 1);
        set(&pieces, 6);
        bitboards[8] = pieces;

        pieces = 0;
        set(&pieces, 2);
        set(&pieces, 5);
        bitboards[9] = pieces;

        pieces = 0;
        set(&pieces, 3);
        bitboards[11] = pieces;

        pieces = 0;
        set(&pieces, 4);
        bitboards[12] = pieces;

        pieces = 0;
        for (int i = 0; i < 16; i++) {
            set(&pieces, i);
        }

        bitboards[13] = pieces;
    }
