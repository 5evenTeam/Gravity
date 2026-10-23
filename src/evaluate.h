#ifndef EVALUATE_H
#define EVALUATE_H

#include "board.h"

namespace Eval {
    int evaluate(const Board& board);
    int evaluate_material(const Board& board);
}

#endif // EVALUATE_H
