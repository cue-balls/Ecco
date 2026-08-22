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
#define NULL 0l
#endif


#include <iostream>
#include <cstdint>
#include <chrono>
#include <memory>
#include <utility>
#include "../lib/httplib.h"
#include "GameState.h"
#include "bitwise.h"

std::vector<std::uint16_t> deb;
std::vector<std::string> de;

unsigned long long count = 0;

std::int16_t alpha_beta(GameState* state, std::int16_t alpha, std::int16_t beta, int depth, std::vector<std::uint64_t>& TT);

constexpr int TABLE_SIZE = 0x4000000;


int main() {
    httplib::Server svr;
    GameState::populate_attacks();
    GameState::generate_hash_keys();

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
        //std::cout << game->see(28, true) << std::endl;
        //game->view_gamestate();
        std::vector<std::uint64_t> transposition_table(TABLE_SIZE, 0x8000000000000000);
        std::cout << req.body << std::endl;
        //std::cout << std::hex << game->hash_key << std::endl;


        std::vector<std::uint16_t> moves = game->get_legal_moves();

        std::string out;
        
        auto start = std::chrono::high_resolution_clock::now();
        if (moves.size() != 0)
        {
            std::int16_t alpha = -32707;
            std::int16_t beta = 32707;

            std::uint16_t best_move;
            std::int16_t best_eval = -32767;



            std::vector<std::pair<std::uint16_t, std::int16_t>> evaluated_moves;

            for (std::uint16_t move : moves) {
                game->make_move(move);
                std::int16_t eval = game->evaluate();
                game->unmake_move(move);

                
                evaluated_moves.push_back({move, eval});
            }

            std::sort(evaluated_moves.begin(), evaluated_moves.end(), [](const auto& a, const auto& b) {
                return a.second < b.second;
            });


            for (int i = 0; i < moves.size(); i++) {
                moves[i] = evaluated_moves[i].first;
            }

            //std::cout << moves.size() << std::endl;

            for (int i = 0; i < moves.size(); i++)
            {
                std::uint16_t m = moves[i];
                game->make_move(m);

                std::int16_t eval;
                std::uint64_t hash_value = transposition_table[(game->hash_key) & 0x3ffffffULL];
                if (!read(hash_value, 63) && clear(hash_value >> 26, 37) == clear(game->hash_key >> 26, 37))
                {
                    eval = hash_value & 0xffff;
                    count++;
                }
                else
                {
                    eval = -alpha_beta(game, -beta, -alpha, 7, transposition_table);
                    std::uint64_t val = static_cast<std::uint64_t>(eval);
                    val |= (clear(game->hash_key >> 26, 37) << 26);
                    transposition_table[(game->hash_key) & 0x3ffffffULL] = val;
                }
                

                game->unmake_move(m);
                


                if (eval > best_eval) {
                    best_eval = eval;
                    best_move = m;
                }

                alpha = std::max((int)alpha, (int)eval);
            }
            //std::cout << std::hex << game->hash_key << std::endl;
            std::cout << count << std::endl;
            count = 0;
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            std::cout << elapsed << std::endl;
            //std::cout << count << std::endl;
            std::cout << (int)best_eval << std::endl;
            
            //game->view_gamestate();

            out += int_to_square(best_move & 63);
            out += int_to_square((best_move >> 6) & 63);

            if (read(best_move, 15)) {
                switch ((best_move >> 12) & 15) {
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
        delete game;
    });
    

    std::cout << "Server listening on 0.0.0.0:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);


    
    //std::uint64_t num = 0;
    //set(&num, 4);
    //set(&num, 1);
    //clear(&num, 4);
    
    //std::cout << read(num, 1) << std::endl;


    //std::uint64_t board = 0;
    //for (int i = 0; i < 16; i++) {
      //  set(&board, i);
    //}

    //view_bitboard(board);
    //std::cout << std::endl;
    
    GameState state("Qnb1kbnr/2pqpppp/1p6/p7/8/2B5/PPP2PPP/R3KBNR b KQk - 0 1");
    GameState::populate_attacks();


    //state.make_move(3388);
    //state.unmake_move(3388);
    //state.make_move(1153);
    //state.make_move(1642);
    //state.make_move(2130);
    //state.make_move(17049);
    //state.make_move(19617);
    //GameState game;
    /*while (1)
    {
        std::string fen;
        std::getline(std::cin, fen);
        GameState game = GameState(fen);
        std::vector<std::uint16_t> moves = game.get_legal_moves();
        game.view_gamestate();
        std::cout << moves.size() << std::endl;
        for (std::uint16_t m : moves)
        {
            std::string from = int_to_square(m & 63);
            std::string to = int_to_square((m >> 6) & 63);
            std::cout << from << to  << " --> " << (int)((m >> 12) & 15) << std::endl;
        }
        view_bitboard(game.bitboards[6] | game.bitboards[13]);
        std::cout << std::endl;
    }*/


    //std::uint64_t king = GameState::piece_attacks[0][45];
    //bishop = occlude_northeast(bishop, state.bitboards[6] | state.bitboards[13], 36);
    //bishop = occlude_northwest(bishop, state.bitboards[6] | state.bitboards[13], 36);
    //bishop = occlude_southeast(bishop, state.bitboards[6] | state.bitboards[13], 36);
    //bishop = occlude_southwest(bishop, state.bitboards[6] | state.bitboards[13], 36);
    //rook = occlude_north(rook, state.bitboards[6] | state.bitboards[13], 36);
    //rook = occlude_south(rook, state.bitboards[6] | state.bitboards[13], 36);
    //rook = occlude_east(rook, state.bitboards[6] | state.bitboards[13], 36);
    //rook = occlude_west(rook, state.bitboards[6] | state.bitboards[13], 36);
    state.view_gamestate();
    //std::uint64_t bishop = (1ULL << 36);
    //view_bitboard(king);
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


    //std::vector<std::uint8_t> white_pawns = serialize(state.bitboards[0]);
    //for (std::uint8_t sq : white_pawns)
    //{
        //std::cout << (int)sq << std::endl;
    //}


    return 0;
}


















std::int16_t alpha_beta(GameState* state, std::int16_t alpha, std::int16_t beta, int depth, std::vector<std::uint64_t>& TT)
{
    //count++;
    if (depth == 0) {
        std::uint64_t inactive_player_composite;
        if (state->white_to_move)
        {
            inactive_player_composite = state->bitboards[13] ^ state->bitboards[12];
        }
        else
        {
            inactive_player_composite = state->bitboards[6] ^ state->bitboards[5];
        }

        std::uint8_t square;
        int hanging_material_bonus = 0;
        while (inactive_player_composite)
        {
            square = bitscan_forward(inactive_player_composite);
            inactive_player_composite = clear(inactive_player_composite, square);


            int delta = state->see(square, state->white_to_move);
            hanging_material_bonus = std::max(hanging_material_bonus, delta);
        }


        return state->evaluate() + hanging_material_bonus;
    }
    
    std::vector<std::uint16_t> legal_moves = state->get_legal_moves();
    if (legal_moves.size() == 0)
    {
        if (state->in_check(state->white_to_move)) {
            return -32000 - depth;
        }

        return 0;
    }


    std::int16_t root_eval = state->evaluate();
    std::int16_t max_eval = -32500;
    
    /*std::vector<std::int16_t> evaluation_list(legal_moves.size());
    for (int i = 0; i < legal_moves.size(); i++)
    {
        state->make_move(legal_moves[i]);
        evaluation_list[i] = state->evaluate();
        state->unmake_move(legal_moves[i]);
    }

    for (int m = 0; m < legal_moves.size(); m++)
    {
        int best_move_index = m;
        std::int16_t best_eval = -32768;

        for (int n = m; n < legal_moves.size(); n++)
        {
            std::int16_t current_eval = evaluation_list[n];

            if (current_eval > best_eval) {
                best_eval = current_eval;
                best_move_index = n;
            }
        }

        std::swap(legal_moves[m], legal_moves[best_move_index]);
        std::swap(evaluation_list[m], evaluation_list[best_move_index]);


        state->make_move(legal_moves[m]);
        std::int16_t eval;
        std::uint64_t hash_value = TT[(state->hash_key) & 0x3ffffffULL];
        if (!read(hash_value, 63) && clear(hash_value >> 26, 37) == clear(state->hash_key >> 26, 37))
        {
            eval = hash_value & 0xffff;
            count++;
        }
        else
        {
            eval = -alpha_beta(state, -beta, -alpha, depth - 1, TT);
            std::uint64_t val = static_cast<std::uint64_t>(eval);
            val |= (clear(state->hash_key >> 26, 37) << 26);
            TT[(state->hash_key) & 0x3ffffffULL] = val;
        }
        state->unmake_move(legal_moves[m]);

        max_eval = std::max((int)max_eval, (int)eval);
        alpha = std::max((int)alpha, (int)eval);

        if (alpha >= beta) {
            break;
        }
    }*/

    std::vector<std::pair<std::uint16_t, std::int16_t>> evaluated_moves;

    for (std::uint16_t move : legal_moves) {
        state->make_move(move);
        std::int16_t eval = state->evaluate();
        state->unmake_move(move);
                
        evaluated_moves.push_back({move, eval});
    }

    std::sort(evaluated_moves.begin(), evaluated_moves.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });


    for (int i = 0; i < legal_moves.size(); i++) {
        legal_moves[i] = evaluated_moves[i].first;
    }



    for (int i = 0; i < legal_moves.size(); i++)
    {
        std::uint16_t m = legal_moves[i];
        state->make_move(m);

        //std::int16_t current_eval = state->evaluate();
        
        //int adj_depth = depth - 1;
        //if (root_eval - current_eval >= 400) {
          //  adj_depth /= 2;
        //}
        
        std::int16_t eval;
        std::uint64_t hash_value = TT[(state->hash_key) & 0x3ffffffULL];
        if (!read(hash_value, 63) && clear(hash_value >> 26, 37) == clear(state->hash_key >> 26, 37))
        {
            eval = hash_value & 0xffff;
            count++;
        }
        else
        {
            eval = -alpha_beta(state, -beta, -alpha, depth - 1, TT);
            std::uint64_t val = static_cast<std::uint64_t>(eval);
            val |= (clear(state->hash_key >> 26, 37) << 26);
            TT[(state->hash_key) & 0x3ffffffULL] = val;
        }



        state->unmake_move(m);



        max_eval = std::max((int)max_eval, (int)eval);
        alpha = std::max((int)alpha, (int)eval);

        if (alpha >= beta) {
            break;
        }
    }

    return max_eval;
}