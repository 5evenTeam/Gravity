#include "bitboard.h"

namespace Bitboards {
    U64 PawnAttacks[COLOR_NB][64];
    U64 KnightAttacks[64];
    U64 KingAttacks[64];
    U64 RayAttacks[8][64]; // 0: N, 1: S, 2: E, 3: W, 4: NE, 5: NW, 6: SE, 7: SW

    enum RayDir {
        DIR_N = 0,
        DIR_S = 1,
        DIR_E = 2,
        DIR_W = 3,
        DIR_NE = 4,
        DIR_NW = 5,
        DIR_SE = 6,
        DIR_SW = 7
    };

    static inline Square msb(U64 b) {
        return static_cast<Square>(63 - __builtin_clzll(b));
    }

    void init() {
        for (int sq = 0; sq < 64; ++sq) {
            int f = square_file(static_cast<Square>(sq));
            int r = square_rank(static_cast<Square>(sq));

            // White Pawn Attacks (moves up-right, up-left)
            U64 wp = 0;
            if (r < 7) {
                if (f > 0) wp |= (1ULL << (sq + 7));
                if (f < 7) wp |= (1ULL << (sq + 9));
            }
            PawnAttacks[WHITE][sq] = wp;

            // Black Pawn Attacks (moves down-right, down-left)
            U64 bp = 0;
            if (r > 0) {
                if (f > 0) bp |= (1ULL << (sq - 9));
                if (f < 7) bp |= (1ULL << (sq - 7));
            }
            PawnAttacks[BLACK][sq] = bp;

            // Knight Attacks
            U64 kn = 0;
            const int knight_moves[8][2] = {
                {1, 2}, {2, 1}, {2, -1}, {1, -2},
                {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}
            };
            for (auto& m : knight_moves) {
                int nf = f + m[0];
                int nr = r + m[1];
                if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
                    kn |= (1ULL << make_square(nf, nr));
                }
            }
            KnightAttacks[sq] = kn;

            // King Attacks
            U64 kg = 0;
            for (int df = -1; df <= 1; ++df) {
                for (int dr = -1; dr <= 1; ++dr) {
                    if (df == 0 && dr == 0) continue;
                    int kf = f + df;
                    int kr = r + dr;
                    if (kf >= 0 && kf < 8 && kr >= 0 && kr < 8) {
                        kg |= (1ULL << make_square(kf, kr));
                    }
                }
            }
            KingAttacks[sq] = kg;

            // Rays
            // 0: North
            U64 rayN = 0;
            for (int nr = r + 1; nr < 8; ++nr) rayN |= (1ULL << make_square(f, nr));
            RayAttacks[DIR_N][sq] = rayN;

            // 1: South
            U64 rayS = 0;
            for (int nr = r - 1; nr >= 0; --nr) rayS |= (1ULL << make_square(f, nr));
            RayAttacks[DIR_S][sq] = rayS;

            // 2: East
            U64 rayE = 0;
            for (int nf = f + 1; nf < 8; ++nf) rayE |= (1ULL << make_square(nf, r));
            RayAttacks[DIR_E][sq] = rayE;

            // 3: West
            U64 rayW = 0;
            for (int nf = f - 1; nf >= 0; --nf) rayW |= (1ULL << make_square(nf, r));
            RayAttacks[DIR_W][sq] = rayW;

            // 4: North-East
            U64 rayNE = 0;
            for (int nf = f + 1, nr = r + 1; nf < 8 && nr < 8; ++nf, ++nr) rayNE |= (1ULL << make_square(nf, nr));
            RayAttacks[DIR_NE][sq] = rayNE;

            // 5: North-West
            U64 rayNW = 0;
            for (int nf = f - 1, nr = r + 1; nf >= 0 && nr < 8; --nf, ++nr) rayNW |= (1ULL << make_square(nf, nr));
            RayAttacks[DIR_NW][sq] = rayNW;

            // 6: South-East
            U64 raySE = 0;
            for (int nf = f + 1, nr = r - 1; nf < 8 && nr >= 0; ++nf, --nr) raySE |= (1ULL << make_square(nf, nr));
            RayAttacks[DIR_SE][sq] = raySE;

            // 7: South-West
            U64 raySW = 0;
            for (int nf = f - 1, nr = r - 1; nf >= 0 && nr >= 0; --nf, --nr) raySW |= (1ULL << make_square(nf, nr));
            RayAttacks[DIR_SW][sq] = raySW;
        }
    }

    static inline U64 ray_attack_pos(Square sq, RayDir dir, U64 occ) {
        U64 ray = RayAttacks[dir][sq];
        U64 blockers = ray & occ;
        if (blockers) {
            Square first = lsb(blockers);
            ray ^= RayAttacks[dir][first];
        }
        return ray;
    }

    static inline U64 ray_attack_neg(Square sq, RayDir dir, U64 occ) {
        U64 ray = RayAttacks[dir][sq];
        U64 blockers = ray & occ;
        if (blockers) {
            Square first = msb(blockers);
            ray ^= RayAttacks[dir][first];
        }
        return ray;
    }

    U64 get_rook_attacks(Square sq, U64 occ) {
        return ray_attack_pos(sq, DIR_N, occ)
             | ray_attack_neg(sq, DIR_S, occ)
             | ray_attack_pos(sq, DIR_E, occ)
             | ray_attack_neg(sq, DIR_W, occ);
    }

    U64 get_bishop_attacks(Square sq, U64 occ) {
        return ray_attack_pos(sq, DIR_NE, occ)
             | ray_attack_pos(sq, DIR_NW, occ)
             | ray_attack_neg(sq, DIR_SE, occ)
             | ray_attack_neg(sq, DIR_SW, occ);
    }
}
