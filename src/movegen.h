#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include "board.h"

struct MoveList {
    Move moves[256];
    int scores[256];
    int count = 0;

    void add(const Move& m) {
        if (count < 256) moves[count++] = m;
    }

    void clear() {
        count = 0;
    }
};

namespace MoveGen {
    void generate_pseudo_legal_moves(const Board& board, MoveList& list, bool only_captures = false);
    void generate_legal_moves(Board& board, MoveList& list);
    void generate_legal_captures(Board& board, MoveList& list);
}

#endif // MOVEGEN_H
