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
#include <random>
#include <cctype>
#include "GameState.h"
#include "bitwise.h"




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

    for (int i = 0; i < 64; i++) {
        mailbox.push_back(255);
    }

    //board setup
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

        //lookup which bitboard to set and set the corresponding composite
        if (std::isalpha(c))
        {
            for (int j = 0; j < 14; j++) 
            {
                if (piece_order[j] == c)
                {
                    bitboards[j] = set(bitboards[j], square);
                    mailbox[square] = j;
                    if (std::isupper(c))
                    {
                        bitboards[6] = set(bitboards[6], square);
                    }
                    else
                    {
                        bitboards[13] = set(bitboards[13], square);
                    }
                }
            }

            square++;
        }
        else
        {
            //adjust square position
            int empty = c - '0';
            square += empty;
        }
    }

    if (i >= FEN.size()) {
        return;
    }

    //determine the active player
    white_to_move = (FEN[++i] == 'w');

    //state stack helps to undo moves
    //stores information about the history of the game so that moves can be unmade
    //moves must be made and unmade in the search function in order to avoid performance issues
    //every time the algorithm moves down through the tree a state is pushed and every time it moves up a state is popped
    //last state always represents the position currently being analyzed
    UndoState current_state {};
    current_state.captured_piece = 255;

    i += 2;
    std::uint8_t castling = 0;

    //tracking castling rights
    //stored as an 8 bit integer
    //first 4 bits represent castling rights (only tracks whether king/rook have moved)
    //0001->white can castle kingside
    //0010->white can castle queenside
    //0100->black can castle kingside
    //1000->black can castle queenside

    while (FEN[i] != ' ')
    {
        char c = FEN[i];
        switch (c) {
            case 'K': castling = set(castling, 0); break;
            case 'Q': castling = set(castling, 1); break;
            case 'k': castling = set(castling, 2); break;
            case 'q': castling = set(castling, 3); break;
        }

        i++;
    }

    current_state.castling_rights = castling;

    //getting en passant flag
    //en passant is marked on the square that the pawn passes over (3rd and 6th ranks) where the capturing pawn lands
    if (FEN[++i] == '-')
    {
        //value outside of 0-63 range represents an empty en passant flag
        current_state.en_passant_square = 255;
    }
    else
    {
        std::string en_passant = FEN.substr(i, i + 2);
        current_state.en_passant_square = square_to_int(en_passant);
    }
    
    current_state.en_passant_square = 43;
    state_stack.push_back(current_state);



    hash_key = 0;
    for (int square = 0; square < 64; square++)
    {
        if (mailbox[square] == 255) {
            continue;
        }


        int piece = mailbox[square];
        if (piece > 6) {
            piece--;
        }


        hash_key ^= zobrist_keys[piece * 64 + square];
    }


    if (white_to_move) {
        hash_key ^= zobrist_keys[768];
    }


    if (current_state.en_passant_square != 255) {
        hash_key ^= zobrist_keys[769 + current_state.en_passant_square % 8];
    }


    hash_key ^= zobrist_keys[777 + castling];
}









void GameState::make_move(std::uint16_t move)
{
    //moves are stored as u16's
    //first 6 bits represent the square piece starts on 0-63
    //next 6 bits represent the square piece lands on 0-63
    //final 4 bits are a special flag containing additional info
    
    //0000->regular move
    //0001->double pawn push
    //0010->kingside castle
    //0011->queenside castle
    //0100->regular capture
    //0101->en passant capture
    //0110/0111 are unused
    //1000->knight promotion
    //1001->bishop promotion
    //1010->rook promotion
    //1011->queen promotion
    //1100->knight promotion with capture
    //1101->bishop promotion with capture
    //1110->rook promotion with capture
    //1111->queen promotion with capture


    //tracking state for undo purposes
    UndoState undo;

    std::uint8_t from_square = move & 63;
    std::uint8_t target_square = (move >> 6) & 63;
    std::uint8_t special_move_data = (move >> 12) & 15;

    //assume same castling rights as previous state and only change if king/rook move
    std::uint8_t castling = state_stack[state_stack.size() - 1].castling_rights;

    //en passant eligibility only lasts for 1 turn and therefore must be reset each move
    std::uint8_t en_passant = 255;
    std::uint8_t capture = mailbox[target_square];
    std::uint8_t moving_piece = mailbox[from_square];

    //color shift is used to differentiate between active and inactive player
    //bitboards[x + color_shift] represents piece x of the active player
    std::uint8_t adjusted_move_id = moving_piece;
    std::uint8_t adjusted_capture_id = capture;
    std::uint8_t adjusted_color_shift = 0;
    std::uint8_t color_shift = 0;
    if (moving_piece > 6) 
    {
        color_shift = 7;
        adjusted_color_shift = 6;
        adjusted_move_id--;  
    }
    else if (read(special_move_data, 2)) 
    {
        adjusted_capture_id--;
    }


    hash_key ^= zobrist_keys[adjusted_move_id * 64 + from_square];
    hash_key ^= zobrist_keys[adjusted_move_id * 64 + target_square];
    hash_key ^= zobrist_keys[768];
    hash_key ^= zobrist_keys[777 + castling];


    //determining changes to castling rights
    if (castling != 0)
    {
        switch (from_square) {
        case 0: castling = clear(castling, 3); break;
        case 7: castling = clear(castling, 2); break;
        case 56: castling = clear(castling, 1); break;
        case 63: castling = clear(castling, 0); break;

        case 4:
            castling = clear(castling, 3);
            castling = clear(castling, 2);
            break;
        
        case 60:
            castling = clear(castling, 1);
            castling = clear(castling, 0);
            break;
        }
    }

    hash_key ^= zobrist_keys[777 + castling];
    if (state_stack[state_stack.size() - 1].en_passant_square != 255) {
        //std::cout << "a" << std::endl;
        hash_key ^= zobrist_keys[769 + state_stack[state_stack.size() - 1].en_passant_square % 8];
    }



    bitboards[moving_piece] ^= ((1ULL << target_square) | (1ULL << from_square));
    mailbox[target_square] = moving_piece;
    mailbox[from_square] = 255;


    bitboards[6 + color_shift] = clear(bitboards[6 + color_shift], from_square);
    bitboards[6 + color_shift] = set(bitboards[6 + color_shift], target_square);
    

    
    if (special_move_data == 1) //setting en passant flag
    {
        en_passant = (target_square + from_square) / 2;
        hash_key ^= zobrist_keys[769 + ((from_square + target_square) / 2) % 8];
    }
    else if (special_move_data == 5) //en passant capture
    {
        if (moving_piece == 0)
        {
            bitboards[7] = clear(bitboards[7], target_square + 8);
            bitboards[13] = clear(bitboards[13], target_square + 8);
            mailbox[target_square + 8] = 255;
            capture = 7;
            hash_key ^= zobrist_keys[6 * 64 + target_square + 8];
        }
        else
        {
            bitboards[0] = clear(bitboards[0], target_square - 8);
            bitboards[6] = clear(bitboards[6], target_square - 8);
            mailbox[target_square - 8] = 255;
            capture = 0;
            hash_key ^= zobrist_keys[target_square - 8];
        }
    }
    else if (read(special_move_data, 2))
    {
        
        bitboards[capture] = clear(bitboards[capture], target_square);
        bitboards[13 - color_shift] = clear(bitboards[13 - color_shift], target_square);
        hash_key ^= zobrist_keys[adjusted_capture_id * 64 + target_square];
    }
    else if (special_move_data == 2) 
    {
        //in the event of a castle, the from square and target square represent the king's movement
        //therefore the only concern is moving the rook and removing all castling rights from the active player
        if (target_square == 62) //white
        {
            bitboards[3] ^= ((1ULL << 63) | (1ULL << 61));
            bitboards[6] = set(bitboards[6], 61);
            bitboards[6] = clear(bitboards[6], 63);
            mailbox[63] = 255;
            mailbox[61] = 3;
            hash_key ^= zobrist_keys[3 * 64 + 61];
            hash_key ^= zobrist_keys[3 * 64 + 63];
        }
        else if (target_square == 6) //black
        {
            bitboards[10] ^= ((1ULL << 7) | (1ULL << 5));
            bitboards[13] = set(bitboards[13], 5);
            bitboards[13] = clear(bitboards[13], 7);
            mailbox[7] = 255;
            mailbox[5] = 10;
            hash_key ^= zobrist_keys[9 * 64 + 5];
            hash_key ^= zobrist_keys[9 * 64 + 7];
        }
    }
    else if (special_move_data == 3)
    {
        if (target_square == 58) //white
        {
            bitboards[3] ^= ((1ULL << 56) | (1ULL << 59));
            bitboards[6] = set(bitboards[6], 59);
            bitboards[6] = clear(bitboards[6], 56);
            mailbox[56] = 255;
            mailbox[59] = 3;
            hash_key ^= zobrist_keys[3 * 64 + 59];
            hash_key ^= zobrist_keys[3 * 64 + 56];
        }
        else if (target_square == 2) //black
        {
            bitboards[10] ^= ((1ULL) | (1ULL << 3));
            bitboards[13] = set(bitboards[13], 3);
            bitboards[13] = clear(bitboards[13], 0);
            mailbox[0] = 255;
            mailbox[3] = 10;
            hash_key ^= zobrist_keys[9 * 64 + 3];
            hash_key ^= zobrist_keys[9 * 64];
        }
    }




    //all promotion moves have the msb set to 1
    //this block handles promotion moves
    if (read(special_move_data, 3))
    {
        switch (special_move_data) {
            
            //knight promotion
            case 8:
            case 12:
                bitboards[0 + color_shift] = clear(bitboards[0 + color_shift], target_square);
                bitboards[1 + color_shift] = set(bitboards[1 + color_shift], target_square);
                mailbox[target_square] = 1 + color_shift;
                hash_key ^= zobrist_keys[adjusted_color_shift * 64 + target_square];
                hash_key ^= zobrist_keys[(1 + adjusted_color_shift) * 64 + target_square];
                break;
            
            //bishop promotion
            case 9:
            case 13:
                bitboards[0 + color_shift] = clear(bitboards[0 + color_shift], target_square);
                bitboards[2 + color_shift] = set(bitboards[2 + color_shift], target_square);
                mailbox[target_square] = 2 + color_shift;
                hash_key ^= zobrist_keys[adjusted_color_shift * 64 + target_square];
                hash_key ^= zobrist_keys[(2 + adjusted_color_shift) * 64 + target_square];
                break;
            
            //rook promotion
            case 10:
            case 14:
                bitboards[0 + color_shift] = clear(bitboards[0 + color_shift], target_square);
                bitboards[3 + color_shift] = set(bitboards[3 + color_shift], target_square);
                mailbox[target_square] = 3 + color_shift;
                hash_key ^= zobrist_keys[adjusted_color_shift * 64 + target_square];
                hash_key ^= zobrist_keys[(3 + adjusted_color_shift) * 64 + target_square];
                break;
            
            //queen promotion
            case 11:
            case 15:
                bitboards[0 + color_shift] = clear(bitboards[0 + color_shift], target_square);
                bitboards[4 + color_shift] = set(bitboards[4 + color_shift], target_square);
                mailbox[target_square] = 4 + color_shift;
                hash_key ^= zobrist_keys[adjusted_color_shift * 64 + target_square];
                hash_key ^= zobrist_keys[(4 + adjusted_color_shift) * 64 + target_square];
                break;
        }
    }


    undo.captured_piece = capture;
    undo.castling_rights = castling;
    undo.en_passant_square = en_passant;
    state_stack.push_back(undo);

    white_to_move = !white_to_move;
}








void GameState::unmake_move(std::uint16_t move)
{
    UndoState undo = state_stack[state_stack.size() - 1];
    state_stack.pop_back();

    std::uint8_t from_square = move & 63;
    std::uint8_t target_square = (move >> 6) & 63;
    std::uint8_t special_move_data = (move >> 12) & 15;
    std::uint8_t capture = undo.captured_piece;
    std::uint8_t moving_piece = mailbox[target_square];

    std::uint8_t adjusted_move_id = moving_piece;
    std::uint8_t adjusted_capture_id = capture;
    std::uint8_t adjusted_color_shift = 0;
    std::uint8_t color_shift = 0;
    if (moving_piece > 6) 
    {
        color_shift = 7;
        adjusted_color_shift = 6;
        adjusted_move_id--;
    }
    else if (read(special_move_data, 2)) 
    {
        adjusted_capture_id--;
    }


    hash_key ^= zobrist_keys[adjusted_move_id * 64 + from_square];
    hash_key ^= zobrist_keys[adjusted_move_id * 64 + target_square];
    hash_key ^= zobrist_keys[768];
    hash_key ^= zobrist_keys[777 + undo.castling_rights];
    hash_key ^= zobrist_keys[777 + state_stack[state_stack.size() - 1].castling_rights];
    
    if (undo.en_passant_square != 255) {
        //std::cout << "c" << std::endl;
        hash_key ^= zobrist_keys[769 + undo.en_passant_square % 8];
    }

    if (state_stack[state_stack.size() - 1].en_passant_square != 255) {
        hash_key ^= zobrist_keys[769 + state_stack[state_stack.size() - 1].en_passant_square % 8];
    }
    

    

    bitboards[moving_piece] ^= ((1ULL << from_square) | (1ULL << target_square));
    mailbox[from_square] = moving_piece;
    mailbox[target_square] = 255;

    bitboards[6 + color_shift] = clear(bitboards[6 + color_shift], target_square);
    bitboards[6 + color_shift] = set(bitboards[6 + color_shift], from_square);




    if (special_move_data < 2) {
        white_to_move = !white_to_move;
        return;
    }
    else if (special_move_data == 5) //undoing en passant
    {
        if (moving_piece == 0)
        {
            bitboards[7] = set(bitboards[7], target_square + 8);
            bitboards[13] = set(bitboards[13], target_square + 8);
            mailbox[target_square + 8] = 7;
            hash_key ^= zobrist_keys[6 * 64 + target_square + 8];
        }
        else
        {
            bitboards[0] = set(bitboards[0], target_square - 8);
            bitboards[6] = set(bitboards[6], target_square - 8);
            mailbox[target_square - 8] = 0;
            hash_key ^= zobrist_keys[target_square - 8];
        }
    }
    else if (read(special_move_data, 2)) 
    {
        bitboards[capture] = set(bitboards[capture], target_square);
        bitboards[13 - color_shift] = set(bitboards[13 - color_shift], target_square);
        mailbox[target_square] = capture;
        hash_key ^= zobrist_keys[adjusted_capture_id * 64 + target_square];
    }
    else if (special_move_data == 2) //putting rooks back if move is a castle
    {
        if (moving_piece < 6)
        {
            bitboards[3] ^= ((1ULL << 61) | (1ULL << 63));
            bitboards[6] = set(bitboards[6], 63);
            bitboards[6] = clear(bitboards[6], 61);
            mailbox[61] = 255;
            mailbox[63] = 3;
            hash_key ^= zobrist_keys[3 * 64 + 61];
            hash_key ^= zobrist_keys[3 * 64 + 63];
        }
        else
        {
            bitboards[10] ^= ((1ULL << 5) | (1ULL << 7));
            bitboards[13] = set(bitboards[13], 7);
            bitboards[13] = clear(bitboards[13], 5);
            mailbox[5] = 255;
            mailbox[7] = 10;
            hash_key ^= zobrist_keys[9 * 64 + 5];
            hash_key ^= zobrist_keys[9 * 64 + 7];
        }
    }
    else if (special_move_data == 3)
    {
        if (moving_piece < 6)
        {
            bitboards[3] ^= ((1ULL << 59) | (1ULL << 56));
            bitboards[6] = set(bitboards[6], 56);
            bitboards[6] = clear(bitboards[6], 59);
            mailbox[59] = 255;
            mailbox[56] = 3;
            hash_key ^= zobrist_keys[3 * 64 + 59];
            hash_key ^= zobrist_keys[3 * 64 + 56];
        }
        else
        {
            bitboards[10] ^= ((1ULL) | (1ULL << 3));
            bitboards[13] = set(bitboards[13], 0);
            bitboards[13] = clear(bitboards[13], 3);
            mailbox[3] = 255;
            mailbox[0] = 10;
            hash_key ^= zobrist_keys[9 * 64 + 3];
            hash_key ^= zobrist_keys[9 * 64];
        }
    }
    
    
    
    
    
    if (read(special_move_data, 3)) //undoing promotion
    {
        switch (special_move_data) {
            case 8:
            case 12:
                bitboards[0 + color_shift] = set(bitboards[0 + color_shift], from_square);
                bitboards[1 + color_shift] = clear(bitboards[1 + color_shift], from_square);
                mailbox[from_square] = color_shift;
                hash_key ^= zobrist_keys[adjusted_color_shift * 64 + from_square];
                hash_key ^= zobrist_keys[(1 + adjusted_color_shift) * 64 + from_square];
                break;
            case 9:
            case 13:
                bitboards[0 + color_shift] = set(bitboards[0 + color_shift], from_square);
                bitboards[2 + color_shift] = clear(bitboards[2 + color_shift], from_square);
                mailbox[from_square] = color_shift;
                hash_key ^= zobrist_keys[adjusted_color_shift * 64 + from_square];
                hash_key ^= zobrist_keys[(2 + adjusted_color_shift) * 64 + from_square];
                break;
            case 10:
            case 14:
                bitboards[0 + color_shift] = set(bitboards[0 + color_shift], from_square);
                bitboards[3 + color_shift] = clear(bitboards[3 + color_shift], from_square);
                mailbox[from_square] = color_shift;
                hash_key ^= zobrist_keys[adjusted_color_shift * 64 + from_square];
                hash_key ^= zobrist_keys[(3 + adjusted_color_shift) * 64 + from_square];
                break;
            case 11:
            case 15:
                bitboards[0 + color_shift] = set(bitboards[0 + color_shift], from_square);
                bitboards[4 + color_shift] = clear(bitboards[4 + color_shift], from_square);
                mailbox[from_square] = color_shift;
                hash_key ^= zobrist_keys[adjusted_color_shift * 64 + from_square];
                hash_key ^= zobrist_keys[(4 + adjusted_color_shift) * 64 + from_square];
                break;
        }
    }


    

    white_to_move = !white_to_move;
}







void GameState::populate_attacks()
{
    for (int i = 0; i < 5; i++) {
        piece_attacks.push_back(std::vector<std::uint64_t>(64));
    }

    std::uint64_t rays = 0;
    for (int square = 0; square < 64; square++)
    {
        for (int p = 0; p < 5; p++)
        {
            switch (p) {
                case 0:
                    rays = 0;
                    rays |= (((1ULL << square) >> 17) & ~Bitwise::HFILE);
                    rays |= (((1ULL << square) >> 15) & ~Bitwise::AFILE);
                    rays |= (((1ULL << square) << 15) & ~Bitwise::HFILE);
                    rays |= (((1ULL << square) << 17) & ~Bitwise::AFILE);
                    rays |= (((1ULL << square) >> 10) & (~Bitwise::HFILE & (~Bitwise::HFILE >> 1)));
                    rays |= (((1ULL << square) << 6) & (~Bitwise::HFILE & (~Bitwise::HFILE >> 1)));
                    rays |= (((1ULL << square) << 10) & (~Bitwise::AFILE & (~Bitwise::AFILE << 1)));
                    rays |= (((1ULL << square) >> 6) & (~Bitwise::AFILE & (~Bitwise::AFILE << 1)));
                    piece_attacks[p][square] = rays;
                    break;
                case 1:
                    rays = 0;
                    rays |= fill_northwest(1ULL << square);
                    rays |= fill_southeast(1ULL << square);
                    rays |= fill_northeast(1ULL << square);
                    rays |= fill_southwest(1ULL << square);
                    rays = clear(rays, square);
                    piece_attacks[p][square] = rays;
                    break;
                case 2:
                    rays = 0;
                    rays |= fill_north(1ULL << square);
                    rays |= fill_south(1ULL << square);
                    rays |= fill_east(1ULL << square);
                    rays |= fill_west(1ULL << square);
                    rays = clear(rays, square);
                    piece_attacks[p][square] = rays;
                    break;
                case 3:
                    rays = 0;
                    rays |= fill_north(1ULL << square);
                    rays |= fill_south(1ULL << square);
                    rays |= fill_east(1ULL << square);
                    rays |= fill_west(1ULL << square);
                    rays |= fill_northwest(1ULL << square);
                    rays |= fill_southeast(1ULL << square);
                    rays |= fill_northeast(1ULL << square);
                    rays |= fill_southwest(1ULL << square);
                    rays = clear(rays, square);
                    piece_attacks[p][square] = rays;
                    break;
                case 4:
                    rays = 0;
                    rays |= (1ULL << square) << 8;
                    rays |= (1ULL << square) >> 8;
                    rays |= ((1ULL << square) << 1) & ~Bitwise::AFILE;
                    rays |= ((1ULL << square) << 9) & ~Bitwise::AFILE;
                    rays |= ((1ULL << square) >> 7) & ~Bitwise::AFILE;
                    rays |= ((1ULL << square) >> 1) & ~Bitwise::HFILE;
                    rays |= ((1ULL << square) >> 9) & ~Bitwise::HFILE;
                    rays |= ((1ULL << square) << 7) & ~Bitwise::HFILE;
                    piece_attacks[p][square] = rays;
                    break;
            }
        }
    }
}







void GameState::generate_hash_keys()
{
    zobrist_keys = std::vector<std::uint64_t>(793);
    std::uint64_t seed = 422715260;
    std::mt19937_64 gen64(seed);

    
    for (int i = 0; i < 793; i++) {
        std::uint64_t tmp = gen64();
        zobrist_keys[i] = tmp;
    }
}






std::uint64_t GameState::get_hash_preview(std::uint16_t move)
{
    UndoState undo = state_stack[state_stack.size() - 1];

    std::uint8_t from_square = move & 63;
    std::uint8_t target_square = (move >> 6) & 63;
    std::uint8_t special_move_data = (move >> 12) & 15;
    std::uint8_t capture = undo.captured_piece;
    std::uint8_t moving_piece = mailbox[target_square];
    std::uint8_t castling = undo.castling_rights;
    std::uint8_t next_hash = hash_key;

    std::uint8_t adjusted_move_id = moving_piece;
    std::uint8_t adjusted_capture_id = capture;
    std::uint8_t adjusted_color_shift = 0;
    std::uint8_t color_shift = 0;
    if (moving_piece > 6) 
    {
        color_shift = 7;
        adjusted_color_shift = 6;
        adjusted_move_id--;
    }
    else if (read(special_move_data, 2)) 
    {
        adjusted_capture_id--;
    }


    next_hash ^= zobrist_keys[adjusted_move_id * 64 + from_square];
    next_hash ^= zobrist_keys[adjusted_move_id * 64 + target_square];
    next_hash ^= zobrist_keys[768];
    next_hash ^= zobrist_keys[777 + castling];


    if (castling != 0)
    {
        switch (from_square) {
        case 0: castling = clear(castling, 3); break;
        case 7: castling = clear(castling, 2); break;
        case 56: castling = clear(castling, 1); break;
        case 63: castling = clear(castling, 0); break;

        case 4:
            castling = clear(castling, 3);
            castling = clear(castling, 2);
            break;
        
        case 60:
            castling = clear(castling, 1);
            castling = clear(castling, 0);
            break;
        }
    }

    next_hash ^= zobrist_keys[777 + castling];
    if (state_stack[state_stack.size() - 1].en_passant_square != 255) {
        next_hash ^= zobrist_keys[769 + state_stack[state_stack.size() - 1].en_passant_square % 8];
    }


    if (special_move_data == 1) //setting en passant flag
    {
        next_hash ^= zobrist_keys[769 + (from_square + target_square / 2) % 8];
    }
    else if (special_move_data == 5) //en passant capture
    {
        if (moving_piece == 0)
        {
            next_hash ^= zobrist_keys[6 * 64 + target_square + 8];
        }
        else
        {
            next_hash ^= zobrist_keys[target_square - 8];
        }
    }
    else if (read(special_move_data, 2))
    {
        next_hash ^= zobrist_keys[adjusted_capture_id * 64 + target_square];
    }
    else if (special_move_data == 2) 
    {
        if (target_square == 62) //white
        {
           next_hash ^= zobrist_keys[3 * 64 + 61];
           next_hash ^= zobrist_keys[3 * 64 + 63];
        }
        else if (target_square == 6) //black
        {
            next_hash ^= zobrist_keys[9 * 64 + 5];
            next_hash ^= zobrist_keys[9 * 64 + 7];
        }
    }
    else if (special_move_data == 3)
    {
        if (target_square == 58) //white
        {
            next_hash ^= zobrist_keys[3 * 64 + 59];
            next_hash ^= zobrist_keys[3 * 64 + 56];
        }
        else if (target_square == 2) //black
        {
            next_hash ^= zobrist_keys[9 * 64 + 3];
            next_hash ^= zobrist_keys[9 * 64];
        }
    }



    if (read(special_move_data, 3))
    {
        switch (special_move_data) {
            
            //knight promotion
            case 8:
            case 12:
                next_hash ^= zobrist_keys[adjusted_color_shift * 64 + target_square];
                next_hash ^= zobrist_keys[(1 + adjusted_color_shift) * 64 + target_square];
                break;
            
            //bishop promotion
            case 9:
            case 13:
                next_hash ^= zobrist_keys[adjusted_color_shift * 64 + target_square];
                next_hash ^= zobrist_keys[(2 + adjusted_color_shift) * 64 + target_square];
                break;
            
            //rook promotion
            case 10:
            case 14:
                next_hash ^= zobrist_keys[adjusted_color_shift * 64 + target_square];
                next_hash ^= zobrist_keys[(3 + adjusted_color_shift) * 64 + target_square];
                break;
            
            //queen promotion
            case 11:
            case 15:
                next_hash ^= zobrist_keys[adjusted_color_shift * 64 + target_square];
                next_hash ^= zobrist_keys[(4 + adjusted_color_shift) * 64 + target_square];
                break;
        }
    }






    return next_hash;
}









bool GameState::in_check(bool white)
{
    int color_shift = 0;
    if (!white) {
        color_shift = 7;
    }

    int king_square = bitscan_forward(bitboards[5 + color_shift]);

    //when searching for checks, we can imagine the king is another piece and look at where that piece would attack
    //those squares can then be checked to see if that piece is there


    std::uint64_t ray = piece_attacks[2][king_square];
    ray = occlude_north(ray, bitboards[6] | bitboards[13], king_square);
    ray = occlude_east(ray, bitboards[6] | bitboards[13], king_square);
    ray = occlude_south(ray, bitboards[6] | bitboards[13], king_square);
    ray = occlude_west(ray, bitboards[6] | bitboards[13], king_square);
    ray &= (bitboards[10 - color_shift] | bitboards[11 - color_shift]);

    if (ray) {
        return true;
    }


    ray = piece_attacks[1][king_square];
    ray = occlude_northeast(ray, bitboards[6] | bitboards[13], king_square);
    ray = occlude_northwest(ray, bitboards[6] | bitboards[13], king_square);
    ray = occlude_southeast(ray, bitboards[6] | bitboards[13], king_square);
    ray = occlude_southwest(ray, bitboards[6] | bitboards[13], king_square);
    ray &= (bitboards[9 - color_shift] | bitboards[11 - color_shift]);

    if (ray) {
        return true;
    }


    //searching for knight checks
    ray = piece_attacks[0][king_square] & bitboards[8 - color_shift];
    if (ray) {
        return true;
    }

    
    ray = piece_attacks[4][king_square] & bitboards[12 - color_shift];
    if (ray) {
        return true;
    }

    if (white)
    {
        if (read((bitboards[7] << 7) & ~Bitwise::HFILE, king_square) || read((bitboards[7] << 9) & ~Bitwise::AFILE, king_square)) {
            return true;
        }
    }
    else
    {
        if (read((bitboards[0] >> 7) & ~Bitwise::AFILE, king_square) || read((bitboards[0] >> 9) & ~Bitwise::HFILE, king_square)) {
            return true;
        }
    }



    return false;
}









std::vector<std::uint16_t> GameState::get_legal_moves()
{
    std::uint8_t color_shift = 7;
    std::uint8_t castle_shift = 0;

    if (white_to_move) {
        color_shift = 0;
        castle_shift = 56;
    }


    std::vector<std::uint16_t> legal_moves;


    //even though it is primarily designed to undo moves, the last element represents the current state
    UndoState undo = state_stack[state_stack.size() - 1];

    std::vector<std::uint8_t> pieces;
    pieces = serialize(bitboards[1 + color_shift]);
    std::vector<std::uint8_t> attacks;
    std::uint16_t move = 0;

    for (std::uint8_t p : pieces)
    {
        std::uint64_t rays = piece_attacks[0][p] & ~bitboards[6 + color_shift];
        attacks = serialize(rays);

        for (std::uint8_t attack : attacks)
        {
            move = (attack << 6) | p;
            if (read(bitboards[13 - color_shift], attack)) {
                move |= (1 << 14);
            }

            if (validate_move(move)) {
                legal_moves.push_back(move);
            }
        }
    }

    pieces = serialize(bitboards[2 + color_shift]);
    for (std::uint8_t p : pieces)
    {
        std::uint64_t rays = piece_attacks[1][p];
        rays = occlude_northeast(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_northwest(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_southeast(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_southwest(rays, bitboards[6] | bitboards[13], p);
        rays &= ~bitboards[6 + color_shift];

        attacks = serialize(rays);

        for (std::uint8_t attack : attacks)
        {
            move = (attack << 6) | p;
            if (read(bitboards[13 - color_shift], attack)) {
                move |= (1 << 14);
            }

            if (validate_move(move)) {
                legal_moves.push_back(move);
            }
        }
    }

    pieces = serialize(bitboards[3 + color_shift]);
    for (std::uint8_t p : pieces)
    {
        std::uint64_t rays = piece_attacks[2][p];
        rays = occlude_north(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_west(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_east(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_south(rays, bitboards[6] | bitboards[13], p);
        rays &= ~bitboards[6 + color_shift];

        attacks = serialize(rays);

        for (std::uint8_t attack : attacks)
        {
            move = (attack << 6) | p;
            if (read(bitboards[13 - color_shift], attack)) {
                move |= (1 << 14);
            }

            if (validate_move(move)) {
                legal_moves.push_back(move);
            }
        }
    }

    pieces = serialize(bitboards[4 + color_shift]);
    for (std::uint8_t p : pieces)
    {
        std::uint64_t rays = piece_attacks[3][p];
        rays = occlude_northeast(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_northwest(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_southeast(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_southwest(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_north(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_west(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_east(rays, bitboards[6] | bitboards[13], p);
        rays = occlude_south(rays, bitboards[6] | bitboards[13], p);
        rays &= ~bitboards[6 + color_shift];

        attacks = serialize(rays);

        for (std::uint8_t attack : attacks)
        {
            move = (attack << 6) | p;
            if (read(bitboards[13 - color_shift], attack)) {
                move |= (1 << 14);
            }

            if (validate_move(move)) {
                legal_moves.push_back(move);
            }
        }
    }

    pieces = serialize(bitboards[5 + color_shift]);

    for (std::uint8_t p : pieces)
    {
        attacks = serialize(piece_attacks[4][p] & ~bitboards[6 + color_shift]);

        for (std::uint8_t attack : attacks)
        {
            move = (attack << 6) | p;
            if (read(bitboards[13 - color_shift], attack)) {
                move |= (1 << 14);
            }

            if (validate_move(move)) {
                legal_moves.push_back(move);
            }
        }
    }
    

    pieces = serialize(bitboards[0 + color_shift]);
    for (std::uint8_t p : pieces)
    {
        std::uint64_t rays = 0;
        if (white_to_move)
        {
            if (!read(bitboards[6], p - 8) && !read(bitboards[13], p - 8)) {
                move = ((p - 8) << 6) | p;
                if (p > 15)
                {
                    if (validate_move(move)) {
                        legal_moves.push_back(move);
                    }
                }
                else
                {
                    for (std::uint16_t i = 8; i < 12; i++)
                    {
                        move |= (i << 12);
                        if (validate_move(move)) {
                            legal_moves.push_back(move);
                        }
                        move &= 0x0FFF;
                    }
                }

                if (p / 8 == 6)
                {
                    if (!read(bitboards[6], p - 16) && !read(bitboards[13], p - 16))
                    {
                        move = ((p - 16) << 6) | p;
                        move |= (1 << 12);
                        if (validate_move(move)) {
                            legal_moves.push_back(move);
                        }
                    }
                }
            }

            rays |= (((1ULL << p) >> 7) & ~Bitwise::AFILE);
            rays |= (((1ULL << p) >> 9) & ~Bitwise::HFILE);

            if (read(rays, undo.en_passant_square))
            {
                move = (undo.en_passant_square << 6) | p;
                move |= (5 << 12);
                if (validate_move(move)) {
                    legal_moves.push_back(move);
                }
            }

            rays &= bitboards[13];
            

            attacks = serialize(rays);

            for (std::uint8_t attack : attacks)
            {
                move = (attack << 6) | p;
                if (p > 15)
                {
                    move |= (1 << 14);
                    if (validate_move(move)) {
                        legal_moves.push_back(move);
                    }
                }
                else
                {
                    for (std::uint16_t i = 12; i < 16; i++)
                    {
                        move |= (i << 12);
                        if (validate_move(move)) {
                            legal_moves.push_back(move);
                        }
                        move &= 0x0FFF;
                    }
                }
            }
        }
        else
        {
            if (!read(bitboards[6], p + 8) && !read(bitboards[13], p + 8)) {
                move = ((p + 8) << 6) | p;
                if (p < 48)
                {
                    if (validate_move(move)) {
                        legal_moves.push_back(move);
                    }
                }
                else
                {
                    for (std::uint16_t i = 8; i < 12; i++)
                    {
                        move |= (i << 12);
                        if (validate_move(move)) {
                            legal_moves.push_back(move);
                        }
                        move &= 0x0FFF;
                    }
                }

                if (p / 8 == 1)
                {
                    if (!read(bitboards[6], p + 16) && !read(bitboards[13], p + 16))
                    {
                        move = ((p + 16) << 6) | p;
                        move |= (1 << 12);
                        if (validate_move(move)) {
                            legal_moves.push_back(move);
                        }
                    }
                }
            }

            rays |= (((1ULL << p) << 7) & ~Bitwise::HFILE);
            rays |= (((1ULL << p) << 9) & ~Bitwise::AFILE);

            if (read(rays, undo.en_passant_square))
            {
                move = (undo.en_passant_square << 6) | p;
                move |= (5 << 12);
                if (validate_move(move)) {
                    legal_moves.push_back(move);
                }
            }

            rays &= bitboards[6];


            attacks = serialize(rays);

            for (std::uint8_t attack : attacks)
            {
                move = (attack << 6) | p;
                if (p < 48)
                {
                    move |= (1 << 14);
                    if (validate_move(move)) {
                        legal_moves.push_back(move);
                    }
                }
                else
                {
                    for (std::uint16_t i = 12; i < 16; i++)
                    {
                        move |= (i << 12);
                        if (validate_move(move)) {
                            legal_moves.push_back(move);
                        }
                        move &= 0x0FFF;
                    }
                }
            }
        }
    }


    

    
    std::uint8_t castling = 0;


    //extracting castling rights by color
    if (white_to_move)
    {
        castling = undo.castling_rights & 3;
    }
    else
    {
        castling = (undo.castling_rights >> 2) & 3;
    }


    if (read(castling, 0)) {
        if (kingside_eligibility()) {
            std::uint16_t move = ((6 + castle_shift) << 6) | (4 + castle_shift);
            move = set(move, 13);
            legal_moves.push_back(move);
        }
    }

    if (read(castling, 1)) {
        if (queenside_eligibility()) {
            std::uint16_t move = ((2 + castle_shift) << 6) | (4 + castle_shift);
            move = set(move, 12);
            move = set(move, 13);
            legal_moves.push_back(move);
        }
    }




    return legal_moves;
}

bool GameState::validate_move(std::uint16_t move)
{
    make_move(move);
    bool validate = !in_check(!white_to_move);
    unmake_move(move);
    return validate;
}

bool GameState::kingside_eligibility()
{
    int castle_shift = 0;
    int color_shift = 7;
    
    if (white_to_move) {
        castle_shift = 56;
        color_shift = 0;
    }

    
    //king position
    if (!read(bitboards[5 + color_shift], 4 + castle_shift)) {
        return false;
    }

    //rook position
    if (!read(bitboards[3 + color_shift], 7 + castle_shift)) {
        return false;
    }

    //in between square
    if (read(bitboards[6], 5 + castle_shift) || read(bitboards[13], 5 + castle_shift)) {
        return false;
    }

    //in between square
    if (read(bitboards[6], 6 + castle_shift) || read(bitboards[13], 6 + castle_shift)) {
        return false;
    }

    //check handling
    for (int i = 4 + castle_shift; i < 7 + castle_shift; i++)
    {
        bitboards[5 + color_shift] = clear(bitboards[5 + color_shift], 4 + castle_shift);
        bitboards[5 + color_shift] = set(bitboards[5 + color_shift], i);

        if (in_check(white_to_move)) {
            bitboards[5 + color_shift] = clear(bitboards[5 + color_shift], i);
            bitboards[5 + color_shift] = set(bitboards[5 + color_shift], 4 + castle_shift);
            return false;
        }

        bitboards[5 + color_shift] = clear(bitboards[5 + color_shift], i);
        bitboards[5 + color_shift] = set(bitboards[5 + color_shift], 4 + castle_shift);
    }




    return true;
}

bool GameState::queenside_eligibility()
{
    int castle_shift = 0;
    int color_shift = 7;
    
    if (white_to_move) {
        castle_shift = 56;
        color_shift = 0;
    }

    //king position
    if (!read(bitboards[5 + color_shift], 4 + castle_shift)) {
        return false;
    }

    //rook position
    if (!read(bitboards[3 + color_shift], castle_shift)) {
        return false;
    }

    //in between square
    if (read(bitboards[6], 1 + castle_shift) || read(bitboards[13], 1 + castle_shift)) {
        return false;
    }

    //in between square
    if (read(bitboards[6], 2 + castle_shift) || read(bitboards[13], 2 + castle_shift)) {
        return false;
    }

    //in between square
    if (read(bitboards[6], 3 + castle_shift) || read(bitboards[13], 3 + castle_shift)) {
        return false;
    }

    //check handling
    for (int i = 2 + castle_shift; i < 5 + castle_shift; i++)
    {
        bitboards[5 + color_shift] = clear(bitboards[5 + color_shift], 4 + castle_shift);
        bitboards[5 + color_shift] = set(bitboards[5 + color_shift], i);

        if (in_check(white_to_move)) {
            bitboards[5 + color_shift] = clear(bitboards[5 + color_shift], i);
            bitboards[5 + color_shift] = set(bitboards[5 + color_shift], 4 + castle_shift);
            return false;
        }

        bitboards[5 + color_shift] = clear(bitboards[5 + color_shift], i);
        bitboards[5 + color_shift] = set(bitboards[5 + color_shift], 4 + castle_shift);
    }


    return true;
}



//debugging tool
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
                board_representation[j] = piece_order[i];
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
