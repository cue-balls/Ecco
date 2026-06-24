//main engine logic
//frontend queries for each move after passing FEN string
//alpha_beta_search attaches a numeric value to each move
//highest value move is passed back to the frontend
//this engine takes the centipawn approach to evaluating positions to avoid floating point math

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <iostream>
#include <cstdint>

#ifdef NULL
#undef NULL
#define NULL 0
#endif


#include <iostream>
#include <cstdint>
#include "../lib/httplib.h"
#include "GameState.h"
#include "bitwise.h"


int main() {
    httplib::Server svr;

    svr.Options(R"(/.*)", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*"); // Change * to your specific frontend URL if needed
        res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.status = 200;
    });
    

    svr.Post("/data", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");

        std::cout << "received" << std::endl;
        GameState game(req.body);

        std::vector<std::uint16_t> moves = game.get_legal_moves();
        std::string out;
        
        if (moves.size() != 0)
        {
            std::uint16_t move = moves[0];

            out += int_to_square(move & 63);
            out += int_to_square((move >> 6) & 63);

            if (read(move, 15)) {
                switch ((move >> 12) & 15) {
                    case 8:
                    case 12:
                        out += 'n';
                        break;
                    case 9:
                    case 13:
                        out += 'b';
                    case 10:
                    case 14:
                        out += 'r';
                    case 11:
                    case 15:
                        out += 'q';
                }
            }
        }
        else
        {
            out = nullptr;
        }

        std::string json = "{\"move\":\"" + out + "\"}";
        res.set_content(json, "text/plain");
        
    });

    std::cout << "Server listening on 0.0.0.0:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);


    /*
    std::uint64_t num = 0;
    set(&num, 4);
    set(&num, 1);
    clear(&num, 4);
    
    //std::cout << read(num, 1) << std::endl;


    std::uint64_t board = 0;
    for (int i = 0; i < 16; i++) {
        set(&board, i);
    }

    //view_bitboard(board);
    //std::cout << std::endl;
    
    GameState state("1r6/p1pNppkp/5P2/4P3/3P4/8/PPP3PP/RNB1K1NR w KQ - 0 1");

    //state.make_move(2745);
    //state.make_move(1153);
    //state.make_move(1642);
    //state.make_move(2130);
    //state.make_move(17049);
    //state.make_move(19617);
    

    state.view_gamestate();
    //std::cout << std::endl;
    //view_bitboard(state.bitboards[6]);
    //std:: cout << std::endl;
    //view_bitboard(state.bitboards[13]);

    //std::cout << state.white_to_move << std::endl;
    //std::cout << (int)(state.state_stack[state.state_stack.size() - 1].castling_rights) << std::endl;
    //std::cout << (int)(state.state_stack[state.state_stack.size() - 1].en_passant_square) << std::endl;
    //std::cout << (state.state_stack[state.state_stack.size() - 1].captured_piece) << std::endl;
    //std::cout << state.square_occupancy(4) << std::endl;
    //std::cout << state.in_check(true) << std::endl;
    
    std::vector<std::uint8_t> rook_moves = state.get_attack_ray('Q', 27);

    for (int i = 0; i < rook_moves.size(); i++)
    {
        //std::cout << (int)rook_moves[i] << std::endl;
    }

    state.white_to_move = !state.white_to_move;
    std::vector<std::uint16_t> moves = state.get_legal_moves();
    std::cout << moves.size() << std::endl;

    for (std::uint16_t move : moves)
    {
        std::string from = int_to_square(move & 63);
        std::string to = int_to_square((move >> 6) & 63);
        std::cout << from << to  << " --> " << (int)((move >> 12) & 15) << std::endl;
    } 

    state.make_move(moves[0]);
    std::cout << state.in_check(false) << std::endl;
*/

    return 0;
}