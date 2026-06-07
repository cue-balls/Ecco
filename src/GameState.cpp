//class to represent the gamestate
//this engine will use the bitboard approach with 14 bitboards to represent the gamestate
//0->white pawns
//1->white knights
//2->white bishops
//3->white rooks
//4->white queens
//5->white king
//6->white pieces composite board
//7->black pawns
//8->black knights
//9->black bishops
//10->black rooks
//11->black queens
//12->black king
//13->black pieces composite board


//square 0 (lsb) represents a8 (top left)
//square 63 (msb) represents h1 (bottom right)

#include <iostream>
#include <vector>
#include <cctype>
#include "GameState.h"
#include "bitwise.h"


//basic conversion functions
int GameState::square_to_int(std::string square) {
    int num = (int)(square[1] - '0');
    int letter_pos = (int)(square[0] - 'a');
    return (8 * (8 - num)) + letter_pos;
}

std::string GameState::int_to_square(int square) {
    std::string out(2, '-');
    char letter = 'a' + (square % 8);
    char num = '0' + (8 - (square / 8));
    out[0] = letter;
    out[1] = num;

    return out;
}
    
//default constructor
//initializes to starting position
GameState::GameState() : bitboards(14), white_to_move(true) {
    
    //pieces is used to construct the bitboards
    //must be reset to 0 each time

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


//arg constructor
//takes FEN string and creates the position from there
//this project will have a frontend that connects to this engine
//on each move js will determine the FEN string for the current position and send it to the backend
//this constructor serves to recreate the gamestate
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

void GameState::view_gamestate() 
{
    std::vector<char> board_representation(64);
        
    for (int i = 0; i < 64; i++) {
        board_representation[i] = '-';
    }

    for (int i = 0; i < 14; i++)
    {
        if (i == 6 || i == 13) {
            continue;
        }

        std::uint64_t board = bitboards[i];
        for (int j = 0; j < 64; j++)
        {
            if (read(board, j)) {
                switch (i) {
                    case 0: board_representation[j] = 'P'; break;
                    case 1: board_representation[j] = 'N'; break;
                    case 2: board_representation[j] = 'B'; break;
                    case 3: board_representation[j] = 'R'; break;
                    case 4: board_representation[j] = 'Q'; break;
                    case 5: board_representation[j] = 'K'; break;
                    case 7: board_representation[j] = 'p'; break;
                    case 8: board_representation[j] = 'n'; break;
                    case 9: board_representation[j] = 'b'; break;
                    case 10: board_representation[j] = 'r'; break;
                    case 11: board_representation[j] = 'q'; break;
                    case 12: board_representation[j] = 'k'; break;
                }
            }
        }
    }

    for (int i = 0; i < 64; i++)
    {
        std::cout << board_representation[i] << '|';
        if ((i + 1) % 8 == 0)
        {
            std::cout << std::endl;
        }
    }
}
