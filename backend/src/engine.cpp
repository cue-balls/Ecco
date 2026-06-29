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


int alpha_beta(GameState* state, int alpha, int beta, int depth);


int main() {
    /*httplib::Server svr;

    svr.Options(R"(/.*)", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.status = 200;
    });
    

    svr.Post("/data", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");

        std::cout << "received" << std::endl;
        GameState * game = new GameState(req.body);

        std::vector<std::uint16_t> moves = game->get_legal_moves();

        std::string out;
        
        if (moves.size() != 0)
        {
            int alpha = -100000;
            int beta = 100000;

            std::uint16_t move;
            int best_eval = -1000000;

            for (std::uint16_t m : moves)
            {
                game->make_move(m);
                int eval = -alpha_beta(game, -beta, -alpha, 7);
                game->unmake_move(m);

                if (eval > best_eval) {
                    best_eval = eval;
                    move = m;
                }

                alpha = std::max(alpha, eval);
            }


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

    //std::cout << "Server listening on 0.0.0.0:8080" << std::endl;
    //svr.listen("0.0.0.0", 8080);
*/

    
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
    
    GameState state("6qk/6qq/4R3/8/2R1R1R1/8/K3R3/8 w - - 0 1");

    state.make_move(18868);
    state.unmake_move(18868);
    //state.make_move(1153);
    //state.make_move(1642);
    //state.make_move(2130);
    //state.make_move(17049);
    //state.make_move(19617);
    
    GameState::populate_attacks();


    std::uint64_t rook = GameState::piece_attacks[3][36];
    rook = occlude_north(rook, state.bitboards[6] | state.bitboards[13], 36);
    rook = occlude_south(rook, state.bitboards[6] | state.bitboards[13], 36);
    rook = occlude_east(rook, state.bitboards[6] | state.bitboards[13], 36);
    rook = occlude_west(rook, state.bitboards[6] | state.bitboards[13], 36);
    //state.view_gamestate();
    //std::uint64_t bishop = (1ULL << 36);
    view_bitboard(rook);
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
    
    //std::vector<std::uint8_t> rook_moves = state.get_attack_ray('Q', 27);

    //for (int i = 0; i < rook_moves.size(); i++)
    //{
        //std::cout << (int)rook_moves[i] << std::endl;
    //}

    //state.white_to_move = !state.white_to_move;
    //std::vector<std::uint16_t> moves = state.get_legal_moves();
    //std::cout << moves.size() << std::endl;

    //for (std::uint16_t move : moves)
    //{
      //  std::string from = int_to_square(move & 63);
        //std::string to = int_to_square((move >> 6) & 63);
        //std::cout << from << to  << " --> " << (int)((move >> 12) & 15) << std::endl;
    //} 

    //std::cout << bitscan_forward(state.bitboards[3]) << std:: endl;
    //std::cout << bitscan_reverse(state.bitboards[3]) << std:: endl;
    //state.make_move(moves[0]);
   // std::cout << state.in_check(false) << std::endl;


    std::vector<std::uint8_t> white_pawns = serialize(state.bitboards[0]);
    for (std::uint8_t sq : white_pawns)
    {
        //std::cout << (int)sq << std::endl;
    }


    return 0;
}


















int alpha_beta(GameState* state, int alpha, int beta, int depth)
{
    if (depth == 0) {
        return state->evaluate();
    }
    
    std::vector<std::uint16_t> legal_moves = state->get_legal_moves();
    if (legal_moves.size() == 0)
    {
        if (state->in_check(state->white_to_move)) {
            return -50000 - depth;
        }

        return 0;
    }


    int root_eval = state->evaluate();
    int max_eval = -100000;
    
    std::vector<std::pair<std::uint16_t, int>> evaluated_moves;

    for (std::uint16_t move : legal_moves) {
        state->make_move(move);
        int eval = state->evaluate();
        state->unmake_move(move);
        
        evaluated_moves.push_back({move, eval});
    }

    std::sort(evaluated_moves.begin(), evaluated_moves.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    for (int i = 0; i < legal_moves.size(); i++) {
        legal_moves[i] = evaluated_moves[i].first;
    }

    for (std::uint16_t m : legal_moves)
    {
        state->make_move(m);
        int current_eval = state->evaluate();
        
        int adj_depth = depth - 1;
        if (root_eval - current_eval >= 400) {
            adj_depth /= 2;
        }
        

        int eval = -alpha_beta(state, -beta, -alpha, adj_depth);


        state->unmake_move(m);

        max_eval = std::max(max_eval, eval);
        alpha = std::max(alpha, eval);

        if (alpha >= beta) {
            break;
        }
    }

    return max_eval;
}