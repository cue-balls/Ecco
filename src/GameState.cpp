//class to represent the gamestate
//this engine will use the bitboard approach with 14 bitboards to represent the gamestate
#include <iostream>
#include <vector>
#include <cctype>
#include "GameState.h"
#include "bitwise.h"
    
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


    GameState::GameState(std::string FEN) : bitboards(14), white_to_move(true) {
        int i;
        for (i = 0; i < 14; i++) {
            bitboards[i] = 0;
        }

        int square = 0;
        for (i = 0; i < FEN.size(); i++)
        {
            char c = FEN[i];
            if (c == ' ') {
                break;
            }

            if (!isalnum(c)) {
                continue;
            }

            if (std::isalpha(c))
            {
                switch (c) {
                    case 'P': 
                        set(&bitboards[0], square);
                        set(&bitboards[6], square);
                        break;
                    case 'N': 
                        set(&bitboards[1], square);
                        set(&bitboards[6], square);
                        break;
                    case 'B': 
                        set(&bitboards[2], square);
                        set(&bitboards[6], square);
                        break;
                    case 'R': 
                        set(&bitboards[3], square);
                        set(&bitboards[6], square);
                        break;
                    case 'Q': 
                        set(&bitboards[4], square);
                        set(&bitboards[6], square);
                        break;
                    case 'K': 
                        set(&bitboards[5], square);
                        set(&bitboards[6], square);
                        break;
                    case 'p': 
                        set(&bitboards[7], square);
                        set(&bitboards[13], square);
                        break;
                    case 'n': 
                        set(&bitboards[8], square);
                        set(&bitboards[13], square);
                        break;
                    case 'b': 
                        set(&bitboards[9], square);
                        set(&bitboards[13], square);
                        break;
                    case 'r': 
                        set(&bitboards[10], square);
                        set(&bitboards[13], square);
                        break;
                    case 'q': 
                        set(&bitboards[11], square);
                        set(&bitboards[13], square);
                        break;
                    case 'k': 
                        set(&bitboards[12], square);
                        set(&bitboards[13], square);
                        break;
                }

                square++;
            }
            else
            {
                int empty = c - '0';
                square += empty;
            }
        }



        white_to_move = (FEN[i + 1] == 'w');
    }
