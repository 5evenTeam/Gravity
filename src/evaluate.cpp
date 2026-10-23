#include "evaluate.h"
#include <algorithm>

namespace Eval {

// PeSTO piece values
const int mg_value[6] = { 82, 337, 365, 477, 1025, 0 };
const int eg_value[6] = { 94, 281, 297, 512,  936, 0 };

// Game phase weights
const int game_phase_weight[6] = { 0, 1, 1, 2, 4, 0 };
const int TOTAL_PHASE = 24;

// PeSTO Midgame Piece-Square Tables (White perspective, rank 0 to rank 7)
const int mg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
     98, 134,  61,  95,  68, 126,  34, -11,
     -6,   7,  26,  31,  65,  56,  25, -20,
    -14,  13,   6,  21,  23,  12,  17, -23,
    -27,  -2,  -5,  12,  17,   6,  10, -25,
    -26,  -4,  -4, -10,   3,   3,  33, -12,
    -35,  -1, -20, -23, -15,  24,  38, -22,
      0,   0,   0,   0,   0,   0,   0,   0,
};

const int eg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8, -10,   7,   0,  12,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
};

const int mg_knight_table[64] = {
   -167, -89, -34, -49,  61, -97, -15, -107,
    -73, -41,  72,  36,  23,  62,   7,  -17,
    -47,  60,  37,  65,  84, 129,  73,   44,
     -9,  17,  19,  53,  37,  69,  18,   22,
    -13,   4,  16,  13,  28,  19,  21,   -8,
    -23,  -9,  12,  10,  19,  17,  25,  -16,
    -29, -53, -12,  -3,  -1,  18, -14,  -19,
   -105, -21, -58, -33, -17, -28, -19,  -23,
};

const int eg_knight_table[64] = {
    -58, -38, -13, -28, -31, -27, -63,  -99,
    -25,  -8, -25,  -2,  -9, -25, -24,  -52,
    -24, -20,  10,   9,  -1,  -9, -19,  -41,
    -17,   3,  22,  22,  22,  11,   8,  -18,
    -18,  -6,  16,  25,  16,  17,   4,  -18,
    -23,  -3,  -1,  15,  10,  -3, -20,  -22,
    -42, -20, -10,  -5,  -2, -20, -23,  -44,
    -29, -51, -23, -15, -22, -18, -50,  -64,
};

const int mg_bishop_table[64] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
};

const int eg_bishop_table[64] = {
    -14, -21, -11,  -8,  -7,  -9, -17, -24,
     -8,  -4,   7, -12,  -3, -13,  -4, -14,
      2,  -8,   0,  -1,  -2,   6,   0,   4,
     -3,   9,  12,   9,  14,  10,   3,   2,
     -6,   3,  13,  19,   7,  10,  -3,  -9,
    -12,  -3,   8,  10,  13,   3,  -7, -15,
    -14, -18,  -7,  -1,   4,  -9, -15, -27,
    -23,  -9, -23,  -5,  -9, -16,  -5, -17,
};

const int mg_rook_table[64] = {
     32,  42,  32,  51,  63,   9,  31,  43,
     27,  32,  58,  62,  80,  67,  26,  44,
     -5,  19,  26,  36,  17,  45,  61,  16,
    -24, -11,   7,  26,  24,  35,  -8, -20,
    -36, -26, -12,  -1,   9,  -7,   6, -23,
    -45, -25, -16, -17,   3,   0,  -5, -33,
    -44, -16, -20,  -9,  -1,  11,  -6, -71,
    -19, -13,   1,  17,  16,   7, -37, -26,
};

const int eg_rook_table[64] = {
     13,  10,  18,  15,  12,  12,   8,   5,
     11,  13,  13,  11,  -3,   3,   8,   3,
      7,   7,   7,   5,   4,  -3,  -5,  -3,
      4,   3,  13,   1,   2,   1,  -1,   2,
      3,   5,   8,   4,  -5,  -6,  -8, -11,
     -4,   0,  -5,  -1,  -7, -12,  -8, -16,
     -6,  -6,   0,   2,  -9,  -9, -11,  -3,
     -9,   2,   3,  -1,  -5, -13,   4, -20,
};

const int mg_queen_table[64] = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
};

const int eg_queen_table[64] = {
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  18,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
};

const int mg_king_table[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

const int eg_king_table[64] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43,
};

const int* mg_tables[6] = { mg_pawn_table, mg_knight_table, mg_bishop_table, mg_rook_table, mg_queen_table, mg_king_table };
const int* eg_tables[6] = { eg_pawn_table, eg_knight_table, eg_bishop_table, eg_rook_table, eg_queen_table, eg_king_table };

inline Square mirror_sq(Square sq) {
    return static_cast<Square>(sq ^ 56);
}

int evaluate(const Board& board) {
    int mg[2] = { 0, 0 };
    int eg[2] = { 0, 0 };
    int game_phase = 0;

    for (int pt = PAWN; pt <= KING; ++pt) {
        // White pieces
        U64 w_pieces = board.pieces_bb(WHITE, static_cast<PieceType>(pt));
        while (w_pieces) {
            Square sq = pop_lsb(w_pieces);
            mg[WHITE] += mg_value[pt] + mg_tables[pt][sq];
            eg[WHITE] += eg_value[pt] + eg_tables[pt][sq];
            game_phase += game_phase_weight[pt];
        }

        // Black pieces
        U64 b_pieces = board.pieces_bb(BLACK, static_cast<PieceType>(pt));
        while (b_pieces) {
            Square sq = pop_lsb(b_pieces);
            Square mirrored = mirror_sq(sq);
            mg[BLACK] += mg_value[pt] + mg_tables[pt][mirrored];
            eg[BLACK] += eg_value[pt] + eg_tables[pt][mirrored];
            game_phase += game_phase_weight[pt];
        }
    }

    // Positional bonuses:
    // Bishop pair
    if (popcount(board.pieces_bb(WHITE, BISHOP)) >= 2) {
        mg[WHITE] += 30;
        eg[WHITE] += 40;
    }
    if (popcount(board.pieces_bb(BLACK, BISHOP)) >= 2) {
        mg[BLACK] += 30;
        eg[BLACK] += 40;
    }

    // Rook on open or semi-open file
    U64 w_pawns = board.pieces_bb(WHITE, PAWN);
    U64 b_pawns = board.pieces_bb(BLACK, PAWN);
    U64 w_rooks = board.pieces_bb(WHITE, ROOK);
    U64 b_rooks = board.pieces_bb(BLACK, ROOK);

    while (w_rooks) {
        Square sq = pop_lsb(w_rooks);
        int file = square_file(sq);
        U64 file_mask = 0x0101010101010101ULL << file;
        if (!(file_mask & w_pawns)) {
            if (!(file_mask & b_pawns)) {
                mg[WHITE] += 25; // Fully open file
                eg[WHITE] += 30;
            } else {
                mg[WHITE] += 15; // Semi-open file
                eg[WHITE] += 15;
            }
        }
    }

    while (b_rooks) {
        Square sq = pop_lsb(b_rooks);
        int file = square_file(sq);
        U64 file_mask = 0x0101010101010101ULL << file;
        if (!(file_mask & b_pawns)) {
            if (!(file_mask & w_pawns)) {
                mg[BLACK] += 25;
                eg[BLACK] += 30;
            } else {
                mg[BLACK] += 15;
                eg[BLACK] += 15;
            }
        }
    }

    // Pawn structure: passed pawns
    U64 wp = w_pawns;
    while (wp) {
        Square sq = pop_lsb(wp);
        int r = square_rank(sq);
        int f = square_file(sq);
        U64 front_mask = 0;
        for (int step_r = r + 1; step_r < 8; ++step_r) {
            front_mask |= (1ULL << make_square(f, step_r));
            if (f > 0) front_mask |= (1ULL << make_square(f - 1, step_r));
            if (f < 7) front_mask |= (1ULL << make_square(f + 1, step_r));
        }
        if (!(front_mask & b_pawns)) {
            // Passed pawn bonus increases dramatically closer to promotion!
            int bonus = (r - 1) * (r - 1) * 6;
            mg[WHITE] += bonus / 2;
            eg[WHITE] += bonus * 2;
        }
    }

    U64 bp = b_pawns;
    while (bp) {
        Square sq = pop_lsb(bp);
        int r = square_rank(sq);
        int f = square_file(sq);
        U64 front_mask = 0;
        for (int step_r = r - 1; step_r >= 0; --step_r) {
            front_mask |= (1ULL << make_square(f, step_r));
            if (f > 0) front_mask |= (1ULL << make_square(f - 1, step_r));
            if (f < 7) front_mask |= (1ULL << make_square(f + 1, step_r));
        }
        if (!(front_mask & w_pawns)) {
            int passed_r = 7 - r;
            int bonus = (passed_r - 1) * (passed_r - 1) * 6;
            mg[BLACK] += bonus / 2;
            eg[BLACK] += bonus * 2;
        }
    }

    // King Shelter & Open File Hazards
    Square wk = board.king_square(WHITE);
    int wk_f = square_file(wk);
    if (square_rank(wk) <= 1) {
        for (int df = -1; df <= 1; ++df) {
            int f = wk_f + df;
            if (f >= 0 && f < 8) {
                U64 file_m = 0x0101010101010101ULL << f;
                if (file_m & w_pawns) mg[WHITE] += 12;
                else mg[WHITE] -= 18;
            }
        }
    }

    Square bk = board.king_square(BLACK);
    int bk_f = square_file(bk);
    if (square_rank(bk) >= 6) {
        for (int df = -1; df <= 1; ++df) {
            int f = bk_f + df;
            if (f >= 0 && f < 8) {
                U64 file_m = 0x0101010101010101ULL << f;
                if (file_m & b_pawns) mg[BLACK] += 12;
                else mg[BLACK] -= 18;
            }
        }
    }

    // Piece Mobility
    U64 occ = board.occupied_bb();
    U64 wn = board.pieces_bb(WHITE, KNIGHT);
    while (wn) {
        Square sq = pop_lsb(wn);
        int mob = popcount(Bitboards::KnightAttacks[sq] & ~board.color_bb(WHITE));
        mg[WHITE] += (mob - 4) * 3;
        eg[WHITE] += (mob - 4) * 3;
    }
    U64 bn = board.pieces_bb(BLACK, KNIGHT);
    while (bn) {
        Square sq = pop_lsb(bn);
        int mob = popcount(Bitboards::KnightAttacks[sq] & ~board.color_bb(BLACK));
        mg[BLACK] += (mob - 4) * 3;
        eg[BLACK] += (mob - 4) * 3;
    }

    U64 wb = board.pieces_bb(WHITE, BISHOP);
    while (wb) {
        Square sq = pop_lsb(wb);
        int mob = popcount(Bitboards::get_bishop_attacks(sq, occ) & ~board.color_bb(WHITE));
        mg[WHITE] += (mob - 7) * 3;
        eg[WHITE] += (mob - 7) * 4;
    }
    U64 bb = board.pieces_bb(BLACK, BISHOP);
    while (bb) {
        Square sq = pop_lsb(bb);
        int mob = popcount(Bitboards::get_bishop_attacks(sq, occ) & ~board.color_bb(BLACK));
        mg[BLACK] += (mob - 7) * 3;
        eg[BLACK] += (mob - 7) * 4;
    }

    int mg_score = mg[WHITE] - mg[BLACK];
    int eg_score = eg[WHITE] - eg[BLACK];

    // Tapered evaluation
    int mg_phase = std::min(game_phase, TOTAL_PHASE);
    int eg_phase = TOTAL_PHASE - mg_phase;

    int eval = (mg_score * mg_phase + eg_score * eg_phase) / TOTAL_PHASE;

    return (board.turn() == WHITE) ? eval : -eval;
}

int evaluate_material(const Board& board) {
    int score = 0;
    for (int pt = PAWN; pt <= QUEEN; ++pt) {
        int w = popcount(board.pieces_bb(WHITE, static_cast<PieceType>(pt)));
        int b = popcount(board.pieces_bb(BLACK, static_cast<PieceType>(pt)));
        score += (w - b) * mg_value[pt];
    }
    return (board.turn() == WHITE) ? score : -score;
}

}
