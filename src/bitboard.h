#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"
#include <iostream>

inline int popcount(U64 b) {
    return __builtin_popcountll(b);
}

inline Square lsb(U64 b) {
    return static_cast<Square>(__builtin_ctzll(b));
}

inline Square pop_lsb(U64& b) {
    Square s = lsb(b);
    b &= b - 1;
    return s;
}

inline U64 square_bb(Square sq) {
    return 1ULL << sq;
}

namespace Bitboards {
    extern U64 PawnAttacks[COLOR_NB][64];
    extern U64 KnightAttacks[64];
    extern U64 KingAttacks[64];
    extern U64 RayAttacks[8][64]; // N, S, E, W, NE, NW, SE, SW

    void init();

    U64 get_bishop_attacks(Square sq, U64 occ);
    U64 get_rook_attacks(Square sq, U64 occ);
    inline U64 get_queen_attacks(Square sq, U64 occ) {
        return get_bishop_attacks(sq, occ) | get_rook_attacks(sq, occ);
    }
}

#endif // BITBOARD_H
