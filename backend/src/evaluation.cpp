#include "GameState.h"
#include "bitwise.h"
#include "PST.h"
#include <vector>


constexpr int TEMPO_BONUS = 15;


std::int16_t GameState::evaluate()
{
    int white_adv = 0;
    int black_adv = 0;


    std::int16_t largest_see = 0;
    std::uint8_t game_phase = 0;
    int white_material = 0;
    int black_material = 0;
    int adjusted_white_material;
    int adjusted_black_material;
    int white_PST = 0;
    int black_PST = 0;


    for (int p = 0; p < 5; p++)
    {
        int piece_count = std::popcount(bitboards[p]);
        white_material += piece_count * piece_value[p];
    }

    for (int p = 7; p < 12; p++)
    {
        int piece_count = std::popcount(bitboards[p]);
        black_material += piece_count * piece_value[p];
    }

    white_adv += white_material;
    black_adv += black_material;


    adjusted_white_material = white_material - (100 * std::popcount(bitboards[0]));
    adjusted_black_material = black_material - (100 * std::popcount(bitboards[7]));


    if (adjusted_white_material + adjusted_black_material < 2800)
    {
        white_adv += white_eg_PST;
        black_adv += black_eg_PST;
    }
    else if (adjusted_white_material + adjusted_black_material <= 5300)
    {
        white_adv += white_mg_PST;
        black_adv += black_mg_PST;
    }
    else
    {
        white_adv += white_opening_PST;
        black_adv += black_opening_PST;
    }


    white_adv += white_pawn_structure;
    black_adv += black_pawn_structure;


    int white_king_safety = 0;
    int black_king_safety = 0;

    std::uint8_t king_square = bitscan_forward(bitboards[5]);
    white_king_safety += 3 * std::popcount(piece_attacks[4][king_square] & ~(bitboards[6] | bitboards[13]));
    
    std::uint64_t rays = piece_attacks[3][king_square];
    rays = occlude_northeast(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_northwest(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_southeast(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_southwest(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_north(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_west(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_east(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_south(rays, bitboards[6] | bitboards[13], king_square);
    rays &= ~bitboards[6];

    white_king_safety -= 2 * std::popcount(rays);
    

    king_square = bitscan_forward(bitboards[12]);
    black_king_safety += 3 * std::popcount(piece_attacks[4][king_square] & ~(bitboards[6] | bitboards[13]));

    rays = piece_attacks[3][king_square];
    rays = occlude_northeast(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_northwest(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_southeast(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_southwest(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_north(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_west(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_east(rays, bitboards[6] | bitboards[13], king_square);
    rays = occlude_south(rays, bitboards[6] | bitboards[13], king_square);
    rays &= ~bitboards[13];

    black_king_safety -= 2 * std::popcount(rays);


    white_adv += white_king_safety;
    black_adv += black_king_safety;


    if (white_to_move) {
        return white_adv + TEMPO_BONUS - black_adv;
    }

    return black_adv + TEMPO_BONUS - white_adv;
}



//calculates estimated material exchange on a square by simulating captures and recaptures
//effectively determines if a sequence of captures wins material on a given square
std::int16_t GameState::see(std::uint8_t square, bool side)
{
    std::int16_t val = 0;
    std::uint8_t attacking_square = get_smallest_attacker(square, side);
    if (attacking_square != 255) {
        val = piece_value[mailbox[square]];
        std::uint16_t move = 0x4000 | (square << 6) | attacking_square;
        make_move(move);
        val = std::max(0, (int)(val - see(square, !side)));
        unmake_move(move);
    }

    return val;
}




std::uint8_t GameState::get_smallest_attacker(std::uint8_t square, bool side)
{
    std::uint8_t color_shift = 0;
    std::uint64_t attacks;
    
    //pawn attacks
    if (side)
    {
        attacks = (bitboards[0] >> 7) & ~Bitwise::AFILE;
        if (read(attacks, square)) {
            std::uint16_t move = 0x4000 | (square << 6) | square + 7;
            if (validate_move(move)) {
                return square + 7;
            }
        }

        attacks = (bitboards[0] >> 9) & ~Bitwise::HFILE;
        if (read(attacks, square)) {
            std::uint16_t move = 0x4000 | (square << 6) | square + 9;
            if (validate_move(move)) {
                return square + 9;
            }
        }
    }
    else
    {
        color_shift = 7;
        
        attacks = (bitboards[13] << 7) & ~Bitwise::HFILE;
        if (read(attacks, square)) {
            std::uint16_t move = 0x4000 | (square << 6) | square - 7;
            if (validate_move(move)) {
                return square - 7;
            }
        }

        attacks = (bitboards[13] << 9) & ~Bitwise::AFILE;
        if (read(attacks, square)) {
            std::uint16_t move = 0x4000 | (square << 6) | square - 9;
            if (validate_move(move)) {
                return square - 9;
            }
        }
    }


    //knight attacks
    attacks = piece_attacks[0][square] & bitboards[1 + color_shift];
    while (attacks) {
        std::uint8_t from_square = bitscan_forward(attacks);
        std::uint16_t move = 0x4000 | (square << 6) | from_square;
        if (validate_move(move)) {
            return from_square;
        }

        attacks = clear(attacks, from_square);
    }


    //bishop attacks
    attacks = piece_attacks[1][square];
    attacks = occlude_northeast(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_southeast(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_northwest(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_southwest(attacks, bitboards[6] | bitboards[13], square);
    attacks &= bitboards[2 + color_shift];

    while (attacks) {
        std::uint8_t from_square = bitscan_forward(attacks);
        std::uint16_t move = 0x4000 | (square << 6) | from_square;
        if (validate_move(move)) {
            return from_square;
        }

        attacks = clear(attacks, from_square);
    }


    //rook attacks
    attacks = piece_attacks[2][square];
    attacks = occlude_north(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_south(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_west(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_east(attacks, bitboards[6] | bitboards[13], square);
    attacks &= bitboards[3 + color_shift];

    while (attacks) {
        std::uint8_t from_square = bitscan_forward(attacks);
        std::uint16_t move = 0x4000 | (square << 6) | from_square;
        if (validate_move(move)) {
            return from_square;
        }

        attacks = clear(attacks, from_square);
    }


    //queen attacks
    attacks = piece_attacks[3][square];
    attacks = occlude_north(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_south(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_west(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_east(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_northeast(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_southeast(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_northwest(attacks, bitboards[6] | bitboards[13], square);
    attacks = occlude_southwest(attacks, bitboards[6] | bitboards[13], square);
    attacks &= bitboards[4 + color_shift];

    while (attacks) {
        std::uint8_t from_square = bitscan_forward(attacks);
        std::uint16_t move = 0x4000 | (square << 6) | from_square;
        if (validate_move(move)) {
            return from_square;
        }

        attacks = clear(attacks, from_square);
    }


    //king attacks
    attacks = piece_attacks[4][square] & bitboards[5 + color_shift];
    if (attacks) {
        std::uint8_t from_square = bitscan_forward(attacks);
        std::uint16_t move = 0x4000 | (square << 6) | from_square;
        if (validate_move(move)) {
            return from_square;
        }
    }



    return 255;
}