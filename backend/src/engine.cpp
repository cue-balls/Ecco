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


std::int16_t alpha_beta(GameState* state, std::int16_t alpha, std::int16_t beta, int depth, std::vector<std::uint64_t>& TT);

unsigned long long count = 0;
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
        std::vector<std::uint64_t> transposition_table(TABLE_SIZE, 0x8000000000000000);
        std::cout << req.body << std::endl;


        std::vector<std::uint16_t> moves = game->get_legal_moves();

        std::string out;
        
        auto start = std::chrono::high_resolution_clock::now();
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
            std::cout << count << std::endl;
            count = 0;
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            std::cout << elapsed << std::endl;
            std::cout << (int)best_eval << std::endl;
            

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

        std::string json = "{\"move\":\"" + out + "\"}";
        res.set_content(json, "text/plain");
        delete game;
    });
    

    std::cout << "Server listening on 0.0.0.0:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}


















std::int16_t alpha_beta(GameState* state, std::int16_t alpha, std::int16_t beta, int depth, std::vector<std::uint64_t>& TT)
{
    if (depth == 0) 
    {
        if (state->in_check(state->white_to_move)) {
            return alpha_beta(state, alpha, beta, 1, TT);
        }
        
        
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