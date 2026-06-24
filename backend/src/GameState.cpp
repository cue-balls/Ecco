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


    
//default constructor
//initializes to starting position
GameState::GameState() : bitboards(14), state_stack(), white_to_move(true) {
    
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

    UndoState current_state {};
    current_state.captured_piece = '-';
    current_state.en_passant_square = 255;
    current_state.castling_rights = 15;

    state_stack.push_back(current_state);
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
                    set(&bitboards[j], square);
                    if (std::isupper(c))
                    {
                        set(&bitboards[6], square);
                    }
                    else
                    {
                        set(&bitboards[13], square);
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
    current_state.captured_piece = '-';

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
            case 'K': set(&castling, 0); break;
            case 'Q': set(&castling, 1); break;
            case 'k': set(&castling, 2); break;
            case 'q': set(&castling, 3); break;
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
    
    state_stack.push_back(current_state);
}






//determines what piece if any is on a given square
char GameState::square_occupancy(std::uint8_t square)
{
    for (int i = 0; i < 14; i++)
    {
        if (i == 6 || i == 13) {
            continue;
        }

        std::uint64_t board = bitboards[i];

        if (read(board, square)) {
            return piece_order[i];
        }
    }


    return '-';
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
    char capture = '-';

    //this loop searches for piece on from square and moves it to target square
    for (int board = 0; board < 14; board++)
    {
        if (board == 6 || board == 13) {
            continue;
        }

        if (read(bitboards[board], from_square))
        {
            //moving piece
            set(&bitboards[board], target_square);
            clear(&bitboards[board], from_square);
            
            if (board < 6) //white piece
            {
                //changing composite board
                clear(&bitboards[6], from_square);
                set(&bitboards[6], target_square);
            }
            else //black piece
            {
                //changing composite board
                clear(&bitboards[13], from_square);
                set(&bitboards[13], target_square);
            }


            //all capture moves by design have the second bit from left set to 1
            //if there is no capture, no further looping is required
            if (!read(special_move_data, 2)) 
            {
                break;
            }
        }
        else if (read(bitboards[board], target_square))
        {
            //handling captured piece
            clear(&bitboards[board], target_square);
            if (board < 6)
            {
                clear(&bitboards[6], target_square);
            }
            else
            {
                clear(&bitboards[13], target_square);
            }
            
            
            capture = piece_order[board];
        }
    }
    


    //determining changes to castling rights
    if (castling != 0)
    {
        switch (from_square) {
        case 0: clear(&castling, 3); break;
        case 7: clear(&castling, 2); break;
        case 56: clear(&castling, 1); break;
        case 63: clear(&castling, 0); break;

        case 4:
            clear(&castling, 3);
            clear(&castling, 2);
            break;
        
        case 60:
            clear(&castling, 1);
            clear(&castling, 0);
            break;
        }
    }
    

    //setting en passant flag
    if (special_move_data == 1) {
        en_passant = (target_square + from_square) / 2;
    }


    //in the event of a castle, the from square and target square represent the king's movement
    //therefore the only concern is moving the rook and removing all castling rights from the active player
    if (special_move_data == 2) 
    {
        if (target_square == 62) //white
        {
            clear(&bitboards[3], 63);
            clear(&bitboards[6], 63);
            set(&bitboards[3], 61);
            set(&bitboards[6], 61);
            clear(&castling, 0);
            clear(&castling, 1);
        }
        else if (target_square == 6) //black
        {
            clear(&bitboards[10], 7);
            clear(&bitboards[13], 7);
            set(&bitboards[10], 5);
            set(&bitboards[13], 5);
            clear(&castling, 2);
            clear(&castling, 3);
        }
    }

    if (special_move_data == 3)
    {
        if (target_square == 58) //white
        {
            clear(&bitboards[3], 56);
            clear(&bitboards[6], 56);
            set(&bitboards[3], 59);
            set(&bitboards[6], 59);
            clear(&castling, 0);
            clear(&castling, 1);
        }
        else if (target_square == 2) //black
        {
            clear(&bitboards[10], 0);
            clear(&bitboards[13], 0);
            set(&bitboards[10], 3);
            set(&bitboards[13], 3);
            clear(&castling, 2);
            clear(&castling, 3);
        }
    }


    //en passant capture
    if (special_move_data == 5) 
    {
        if (target_square / 8 == 2)
        {
            clear(&bitboards[7], target_square + 8);
            clear(&bitboards[13], target_square + 8);
            capture = 'p';
        }
        else if (target_square / 8 == 5)
        {
            clear(&bitboards[0], target_square - 8);
            clear(&bitboards[6], target_square - 8);
            capture = 'P';
        }
    }




    //all promotion moves have the msb set to 1
    //this block handles promotion moves
    if (read(special_move_data, 3))
    {
        //color shift is used to differentiate between active and inactive player
        //bitboards[x + color_shift] represents piece x of the active player
        int color_shift = 0;
        if (target_square >= 56) {
            color_shift = 7;
        }

        switch (special_move_data) {
            
            //knight promotion
            case 8:
            case 12:
                clear(&bitboards[0 + color_shift], target_square);
                set(&bitboards[1 + color_shift], target_square);
                break;
            
            //bishop promotion
            case 9:
            case 13:
                clear(&bitboards[0 + color_shift], target_square);
                set(&bitboards[2 + color_shift], target_square);
                break;
            
            //rook promotion
            case 10:
            case 14:
                clear(&bitboards[0 + color_shift], target_square);
                set(&bitboards[3 + color_shift], target_square);
                break;
            
            //queen promotion
            case 11:
            case 15:
                clear(&bitboards[0 + color_shift], target_square);
                set(&bitboards[4 + color_shift], target_square);
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
    std::uint8_t captured_board;

    if (read(special_move_data, 2))
    {
        for (int i = 0; i < 14; i++)
        {
            if (piece_order[i] == undo.captured_piece) {
                captured_board = i;
            }
        }
    }
    
    int board;
    for (board = 0; board < 14; board++)
    {
        if (board == 6 || board == 13) {
            continue;
        }

        if (read(bitboards[board], target_square))
        {
            clear(&bitboards[board], target_square);
            set(&bitboards[board], from_square);
            
            //updating composite
            if (board < 6) 
            {
                clear(&bitboards[6], target_square);
                set(&bitboards[6], from_square);
            }
            else
            {
                clear(&bitboards[13], target_square);
                set(&bitboards[13], from_square);
            }


            //handling non en passant captures
            if (read(special_move_data, 2) && special_move_data != 5) 
            {
                set(&bitboards[captured_board], target_square);

                if (captured_board < 6)
                {
                    set(&bitboards[6], target_square);
                }
                else
                {
                    set(&bitboards[13], target_square);
                }
            }

            break;
        }
    }



    //putting rooks back if move is a castle
    if (special_move_data == 2)
    {
        if (board < 6)
        {
            clear(&bitboards[3], 61);
            clear(&bitboards[6], 61);
            set(&bitboards[3], 63);
            set(&bitboards[6], 63);
        }
        else
        {
            clear(&bitboards[10], 5);
            clear(&bitboards[13], 5);
            set(&bitboards[10], 7);
            set(&bitboards[13], 7);
        }
    }

    if (special_move_data == 3)
    {
        if (board < 6)
        {
            clear(&bitboards[3], 59);
            clear(&bitboards[6], 59);
            set(&bitboards[3], 56);
            set(&bitboards[6], 56);
        }
        else
        {
            clear(&bitboards[10], 3);
            clear(&bitboards[13], 3);
            set(&bitboards[10], 0);
            set(&bitboards[13], 0);
        }
    }


    //undoing en passant capture
    if (special_move_data == 5)
    {
        if (board < 6)
        {
            set(&bitboards[7], target_square + 8);
            set(&bitboards[13], target_square + 8);
        }
        else
        {
            set(&bitboards[0], target_square - 8);
            set(&bitboards[6], target_square - 8);
        }
    }

    std::uint8_t color_shift = 0;
    if (board > 6) {
        color_shift = 7;
    }


    //undoing promotion
    if (read(special_move_data, 3))
    {
        switch (special_move_data) {
            case 8:
            case 12:
                set(&bitboards[0 + color_shift], from_square);
                clear(&bitboards[1 + color_shift], from_square);
                break;
            case 9:
            case 13:
                set(&bitboards[0 + color_shift], from_square);
                clear(&bitboards[2 + color_shift], from_square);
                break;
            case 10:
            case 14:
                set(&bitboards[0 + color_shift], from_square);
                clear(&bitboards[3 + color_shift], from_square);
                break;
            case 11:
            case 15:
                set(&bitboards[0 + color_shift], from_square);
                clear(&bitboards[4 + color_shift], from_square);
                break;
        }
    }

    white_to_move = !white_to_move;
}







//takes an input piece and generates all squares it exerts pressure on
//includes all squares that can be moved to and all same color pieces it is defending
//pawn moves are more complicated and are calculated separately
std::vector<std::uint8_t> GameState::get_attack_ray(char attacking_piece, std::uint8_t origin)
{
    std::vector<std::uint8_t> ray;

    //mod and floor division operators are routinely used to get rank/file info
    //this is useful when a piece being on the edge of the board would affect its movement


    if (attacking_piece == 'N' || attacking_piece == 'n')
    {
        //knight moves can be calculated by shifting the square the knight is on by a value
        //knight shift values are 6, 10, 15, 17, -6, -10, -15, -17
        //however we must check to make sure the knight is not on the edge of the board to verify each shift

        if (origin > 15)
        {
            if (origin % 8 != 0) {
                ray.push_back(origin - 17);
            }

            if (origin % 8 != 7) {
                ray.push_back(origin - 15);
            }
        }

        if (origin < 48)
        {
            if (origin % 8 != 0) {
                ray.push_back(origin + 15);
            }

            if (origin % 8 != 7) {
                ray.push_back(origin + 17);
            }
        }

        if (origin > 7)
        {
            if (origin % 8 > 1) {
                ray.push_back(origin - 10);
            }

            if (origin % 8 < 6) {
                ray.push_back(origin - 6);
            }
        }

        if (origin < 56)
        {
            if (origin % 8 > 1) {
                ray.push_back(origin + 6);
            }

            if (origin % 8 < 6) {
                ray.push_back(origin + 10);
            }
        }
    }
    else if (attacking_piece == 'B' || attacking_piece == 'b')
    {
        //diagonals are always shifts of 7 or 9
        //edge detection must be done


        std::uint8_t attacking_square = origin;

        while (attacking_square > 7 && attacking_square % 8 != 0)
        {
            attacking_square -= 9;
            ray.push_back(attacking_square);
            

            //if there is a piece in the way the diagonal stops being calculated
            if (read(bitboards[6], attacking_square) || read(bitboards[13], attacking_square)) {
                break;
            }
        }

        
        
        attacking_square = origin;
        while (attacking_square > 7 && attacking_square % 8 != 7)
        {
            attacking_square -= 7;
            ray.push_back(attacking_square);
            
            if (read(bitboards[6], attacking_square) || read(bitboards[13], attacking_square)) {
                break;
            }
        }

        
        
        attacking_square = origin;
        while (attacking_square < 56 && attacking_square % 8 != 0)
        {
            attacking_square += 7;
            ray.push_back(attacking_square);
            
            if (read(bitboards[6], attacking_square) || read(bitboards[13], attacking_square)) {
                break;
            }
        }

        
        
        attacking_square = origin;
        while (attacking_square < 56 && attacking_square % 8 != 7)
        {
            attacking_square += 9;
            ray.push_back(attacking_square);
            
            if (read(bitboards[6], attacking_square) || read(bitboards[13], attacking_square)) {
                break;
            }
        }
    }
    else if (attacking_piece == 'R' || attacking_piece == 'r')
    {
        std::uint8_t attacking_square = origin;

        while (attacking_square % 8 != 0) //file shift
        {
            attacking_square--;
            ray.push_back(attacking_square);
            
            if (read(bitboards[6], attacking_square) || read(bitboards[13], attacking_square)) {
                break;
            }
        }



        attacking_square = origin;
        while (attacking_square % 8 != 7) //file shift
        {
            attacking_square++;
            ray.push_back(attacking_square);
            
            if (read(bitboards[6], attacking_square) || read(bitboards[13], attacking_square)) {
                break;
            }
        }



        attacking_square = origin;
        while (attacking_square > 7) //rank shift
        {
            attacking_square -= 8;
            ray.push_back(attacking_square);
            
            if (read(bitboards[6], attacking_square) || read(bitboards[13], attacking_square)) {
                break;
            }
        }

        
        
        attacking_square = origin;
        while (attacking_square < 56) //rank shift
        {
            attacking_square += 8;
            ray.push_back(attacking_square);
            
            if (read(bitboards[6], attacking_square) || read(bitboards[13], attacking_square)) {
                break;
            }
        }
    }
    else if (attacking_piece == 'Q' || attacking_piece == 'q')
    {
        //queen rays are recursive combinations of bishop and rook rays

        std::vector<std::uint8_t> diagonal = get_attack_ray(piece_order[2], origin);
        std::vector<std::uint8_t> orthogonal = get_attack_ray(piece_order[3], origin);
        diagonal.reserve(diagonal.size() + orthogonal.size());
        diagonal.insert(diagonal.end(), orthogonal.begin(), orthogonal.end());

        return diagonal;
    }
    else if (attacking_piece == 'K' || attacking_piece == 'k')
    {
        if (origin > 7)
        {
            ray.push_back(origin - 8);
            if (origin % 8 != 0) ray.push_back(origin - 9);
            if (origin % 8 != 7) ray.push_back(origin - 7);
        }

        if (origin < 56)
        {
            ray.push_back(origin + 8);
            if (origin % 8 != 0) ray.push_back(origin + 7);
            if (origin % 8 != 7) ray.push_back(origin + 9);
        }

        if (origin % 8 != 0) ray.push_back(origin - 1);
        if (origin % 8 != 7) ray.push_back(origin + 1);
    }


    return ray;
}






bool GameState::in_check(bool white)
{
    int color_shift = 0;
    if (!white) {
        color_shift = 7;
    }

    int king_square;

    //looping to find where king is
    for (int square = 0; square < 64; square++)
    {
        if (read(bitboards[5 + color_shift], square))
        {
            king_square = square;
            break;
        }
    }

    //when searching for checks, we can imagine the king is another piece and look at where that piece would attack
    //those squares can then be checked to see if that piece is there


    //searching for knight checks
    std::vector<std::uint8_t> ray = get_attack_ray(piece_order[1], king_square);
    for (std::uint8_t square : ray)
    {
        if (read(bitboards[8 - color_shift], square)) {
            return true;
        }
    }



    //pawn checks are separate
    if (white)
    {
        if (king_square % 8 != 0 && read(bitboards[7], king_square - 9)) {
            return true;
        }

        if (king_square % 8 != 7 && read(bitboards[7], king_square - 7)) {
            return true;
        }
    }
    else
    {
        if (king_square % 8 != 0 && read(bitboards[0], king_square + 7)) {
            return true;
        }

        if (king_square % 8 != 7 && read(bitboards[0], king_square + 9)) {
            return true;
        }
    }



    //a king can never actually give check, but for the sake of validating moves, it must be accounted for
    ray = get_attack_ray(piece_order[5], king_square);
    for (std::uint8_t square : ray)
    {
        if (read(bitboards[12 - color_shift], square)) {
            return true;
        }
    }


    //looks at the orthogonal and diagonal rays stemming from the king
    //in other words we think of our king like a queen and check the squares it sees
    ray = get_attack_ray(piece_order[4], king_square);
    for (std::uint8_t square : ray)
    {
        if (!read(bitboards[13 - color_shift], square)) {
            continue;
        }

        if (read(bitboards[11 - color_shift], square)) {
            return true;
        }


        //differentiating between rook and bishop
        std::uint8_t difference = std::max((int)king_square, (int)square) - std::min((int)king_square, (int)square);

        //orthogonal check
        if (difference % 8 == 0 && read(bitboards[10 - color_shift], square)) {
            return true;
        }

        //orthogonal check
        if ((square / 8) == (king_square / 8) && read(bitboards[10 - color_shift], square)) {
            return true;
        }

        //diagonal check
        if (difference % 8 != 0 && (square / 8) != (king_square / 8) && read(bitboards[9 - color_shift], square)) {
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


    for (int from_square = 0; from_square < 64; from_square++)
    {
        if (!read(bitboards[6 + color_shift], from_square)) {
            continue;
        }


        if (!read(bitboards[0 + color_shift], from_square))
        {
            std::uint8_t piece = 0;
            for (int i = 1 + color_shift; i < 6 + color_shift; i++)
            {
                if (read(bitboards[i], from_square)) {
                    piece = i;
                    break;
                }
            }

            //attack ray is used to find available moves
            //however it includes moves in which a player would capture there own piece, which must be filtered out
            std::vector<std::uint8_t> ray = get_attack_ray(piece_order[piece], from_square);

            for (std::uint8_t target : ray)
            {
                if (read(bitboards[6 + color_shift], target)) {
                    continue;
                }

                //move packing
                std::uint16_t move = ((std::uint16_t)target << 6) | from_square;
                if (read(bitboards[13 - color_shift], target)) {
                    //set the capture flag
                    set(&move, 14);
                }

                //last step in move validation is to determine if king would be vulnerable if the move were played
                //since making and unmaking moves is already necessary it can be repurposed here

                if (validate_move(move)) legal_moves.push_back(move);
            }
        }



        //white pawn moves
        if (white_to_move && read(bitboards[0], from_square)) 
        {
            //single pawn push
            if (!read(bitboards[6], from_square - 8) && !read(bitboards[13], from_square - 8)) 
            {
                std::uint16_t move = ((from_square - 8) << 6) | from_square;
                
                //promotion handling
                if ((from_square - 8) / 8 == 0)
                {
                    for (int i = 8; i < 12; i++)
                    {
                        move |= (i << 12);
                        if (validate_move(move)) legal_moves.push_back(move);
                        move &= 4095;
                    }
                }
                else
                {
                    if (validate_move(move)) legal_moves.push_back(move);
                }
            }


            //double pawn push
            if (from_square / 8 == 6)
            {
                if (!read(bitboards[6], from_square - 8) && !read(bitboards[13], from_square - 8) 
                && !read(bitboards[6], from_square - 16) && !read(bitboards[13], from_square - 16))
                {
                    std::uint16_t move = ((from_square - 16) << 6) | from_square;
                    set(&move, 12);
                    if (validate_move(move)) legal_moves.push_back(move);
                }
            }


            //diagonal attacks
            if (from_square % 8 != 0) 
            {
                if (read(bitboards[13], from_square - 9))
                {
                    std::uint16_t move = ((from_square - 9) << 6) | from_square;
                
                    //promotion handling
                    if ((from_square - 9) / 8 == 0)
                    {
                        for (int i = 12; i < 16; i++)
                        {
                            move |= (i << 12);
                            if (validate_move(move)) legal_moves.push_back(move);
                            move &= 4095;
                        }
                    }
                    else
                    {
                        set(&move, 14);
                        if (validate_move(move)) legal_moves.push_back(move);
                    }
                }


                //en passant handling
                if (from_square - 9 == undo.en_passant_square && from_square / 8 == 3)
                {
                    std::uint16_t move = ((from_square - 9) << 6) | from_square;
                    set(&move, 14);
                    set(&move, 12);
                    if (validate_move(move)) legal_moves.push_back(move);
                }
            }


            //diagonal attacks
            if (from_square % 8 != 7) 
            {
                if (read(bitboards[13], from_square - 7))
                {
                    std::uint16_t move = ((from_square - 7) << 6) | from_square;
                
                    //promotion handling
                    if ((from_square - 7) / 8 == 0)
                    {
                        for (int i = 12; i < 16; i++)
                        {
                            move |= (i << 12);
                            if (validate_move(move)) legal_moves.push_back(move);
                            move &= 4095;
                        }
                    }
                    else
                    {
                        set(&move, 14);
                        if (validate_move(move)) legal_moves.push_back(move);
                    }
                }


                //en passant handling
                if (from_square - 7 == undo.en_passant_square && from_square / 8 == 3)
                {
                    std::uint16_t move = ((from_square - 7) << 6) | from_square;
                    set(&move, 14);
                    set(&move, 12);
                    if (validate_move(move)) legal_moves.push_back(move);
                }
            }
        }

        
        //black pawn moves
        if (!white_to_move && read(bitboards[7], from_square))
        {
            //single pawn push
            if (!read(bitboards[6], from_square + 8) && !read(bitboards[13], from_square + 8)) 
            {
                std::uint16_t move = ((from_square + 8) << 6) | from_square;
                if ((from_square + 8) / 8 == 7)
                {
                    for (int i = 8; i < 12; i++)
                    {
                        move |= (i << 12);
                        if (validate_move(move)) legal_moves.push_back(move);
                        move &= 4095;
                    }
                }
                else
                {
                    if (validate_move(move)) legal_moves.push_back(move);
                }
            }


            //double pawn push
            if (from_square / 8 == 1)
            {
                if (!read(bitboards[6], from_square + 8) && !read(bitboards[13], from_square + 8) 
                && !read(bitboards[6], from_square + 16) && !read(bitboards[13], from_square + 16))
                {
                    std::uint16_t move = ((from_square + 16) << 6) | from_square;
                    set(&move, 12);
                    if (validate_move(move)) legal_moves.push_back(move);
                }
            }

            //diagonal attacks
            if (from_square % 8 != 0) 
            {
                if (read(bitboards[6], from_square + 7))
                {
                    std::uint16_t move = ((from_square + 7) << 6) | from_square;
                
                    if ((from_square + 7) / 8 == 7)
                    {
                        for (int i = 12; i < 16; i++)
                        {
                            move |= (i << 12);
                            if (validate_move(move)) legal_moves.push_back(move);
                            move &= 4095;
                        }
                    }
                    else
                    {
                        set(&move, 14);
                        if (validate_move(move)) legal_moves.push_back(move);
                    }
                }


                //en passant
                if (from_square + 7 == undo.en_passant_square && from_square / 8 == 4)
                {
                    std::uint16_t move = ((from_square + 7) << 6) | from_square;
                    set(&move, 14);
                    set(&move, 12);
                    if (validate_move(move)) legal_moves.push_back(move);
                }
                
            }

            //diagonal attacks
            if (from_square % 8 != 7) 
            {
                if (read(bitboards[6], from_square + 9))
                {
                    std::uint16_t move = ((from_square + 9) << 6) | from_square;
                
                    if ((from_square + 9) / 8 == 7)
                    {
                        for (int i = 12; i < 16; i++)
                        {
                            move |= (i << 12);
                            if (validate_move(move)) legal_moves.push_back(move);
                            move &= 4095;
                        }
                    }
                    else
                    {
                        set(&move, 14);
                        if (validate_move(move)) legal_moves.push_back(move);
                    }
                }


                //en passant
                if (from_square + 9 == undo.en_passant_square && from_square / 8 == 4)
                {
                    std::uint16_t move = ((from_square + 9) << 6) | from_square;
                    set(&move, 14);
                    set(&move, 12);
                    if (validate_move(move)) legal_moves.push_back(move);
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
            set(&move, 13);
            legal_moves.push_back(move);
        }
    }

    if (read(castling, 1)) {
        if (queenside_eligibility()) {
            std::uint16_t move = ((2 + castle_shift) << 6) | (4 + castle_shift);
            set(&move, 12);
            set(&move, 13);
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
        clear(&bitboards[5 + color_shift], 4 + castle_shift);
        set(&bitboards[5 + color_shift], i);

        if (in_check(white_to_move)) {
            return false;
        }

        clear(&bitboards[5 + color_shift], i);
        set(&bitboards[5 + color_shift], 4 + castle_shift);
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
        clear(&bitboards[5 + color_shift], 4 + castle_shift);
        set(&bitboards[5 + color_shift], i);

        if (in_check(white_to_move)) {
            return false;
        }

        clear(&bitboards[5 + color_shift], i);
        set(&bitboards[5 + color_shift], 4 + castle_shift);
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
