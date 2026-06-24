#include "GameState.h"
#include "bitwise.h"
#include <vector>

int GameState::evaluate()
{
    int white_adv = 0;
    int black_adv = 0;
    
    for (int p = 0; p < 5; p++)
    {
        int piece_count = std::popcount(bitboards[p]);
        white_adv += piece_count * piece_value[p];
    }

    for (int p = 7; p < 12; p++)
    {
        int piece_count = std::popcount(bitboards[p]);
        black_adv += piece_count * piece_value[p];
    }




    if (white_to_move) {
        return white_adv - black_adv;
    }

    return black_adv - white_adv;
}