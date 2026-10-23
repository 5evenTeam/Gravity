#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "types.h"

namespace Zobrist {
    extern U64 PieceSquare[PIECE_NB][64];
    extern U64 SideToMove;
    extern U64 Castling[16];
    extern U64 EnPassant[8];

    void init();
}

#endif // ZOBRIST_H
