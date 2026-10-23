#include "zobrist.h"

namespace Zobrist {
    U64 PieceSquare[PIECE_NB][64];
    U64 SideToMove;
    U64 Castling[16];
    U64 EnPassant[8];

    static U64 rand64(U64& state) {
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        return state * 2685821657736338717ULL;
    }

    void init() {
        U64 state = 1070372;
        for (int p = 0; p < PIECE_NB; ++p) {
            for (int sq = 0; sq < 64; ++sq) {
                PieceSquare[p][sq] = rand64(state);
            }
        }
        SideToMove = rand64(state);
        for (int i = 0; i < 16; ++i) {
            Castling[i] = rand64(state);
        }
        for (int i = 0; i < 8; ++i) {
            EnPassant[i] = rand64(state);
        }
    }
}
