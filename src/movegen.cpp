#include "movegen.h"

namespace MoveGen {

    void generate_pseudo_legal_moves(const Board& board, MoveList& list, bool only_captures) {
        Color us = board.turn();
        Color them = ~us;
        U64 our_pieces = board.color_bb(us);
        U64 their_pieces = board.color_bb(them);
        U64 occ = our_pieces | their_pieces;
        U64 empty = ~occ;

        // Pawns
        U64 pawns = board.pieces_bb(us, PAWN);
        int push_offset = (us == WHITE) ? 8 : -8;
        int promo_rank = (us == WHITE) ? 7 : 0;
        int start_rank = (us == WHITE) ? 1 : 6;

        while (pawns) {
            Square from = pop_lsb(pawns);
            int r = square_rank(from);

            // Single Push & Double Push
            if (!only_captures) {
                Square to_single = static_cast<Square>(from + push_offset);
                if (to_single >= 0 && to_single < 64 && !(occ & (1ULL << to_single))) {
                    if (square_rank(to_single) == promo_rank) {
                        list.add(Move(from, to_single, FLAG_PROMO_Q));
                        list.add(Move(from, to_single, FLAG_PROMO_R));
                        list.add(Move(from, to_single, FLAG_PROMO_B));
                        list.add(Move(from, to_single, FLAG_PROMO_N));
                    } else {
                        list.add(Move(from, to_single, FLAG_QUIET));

                        // Double Push
                        if (r == start_rank) {
                            Square to_double = static_cast<Square>(from + 2 * push_offset);
                            if (!(occ & (1ULL << to_double))) {
                                list.add(Move(from, to_double, FLAG_DOUBLE_PAWN));
                            }
                        }
                    }
                }
            }

            // Normal Pawn Captures
            U64 attacks = Bitboards::PawnAttacks[us][from] & their_pieces;
            while (attacks) {
                Square to = pop_lsb(attacks);
                if (square_rank(to) == promo_rank) {
                    list.add(Move(from, to, FLAG_PROMO_CAP_Q));
                    list.add(Move(from, to, FLAG_PROMO_CAP_R));
                    list.add(Move(from, to, FLAG_PROMO_CAP_B));
                    list.add(Move(from, to, FLAG_PROMO_CAP_N));
                } else {
                    list.add(Move(from, to, FLAG_CAPTURE));
                }
            }

            // En Passant
            Square ep = board.ep();
            if (ep != NO_SQ) {
                if (Bitboards::PawnAttacks[us][from] & (1ULL << ep)) {
                    list.add(Move(from, ep, FLAG_EN_PASSANT));
                }
            }
        }

        // Knights
        U64 knights = board.pieces_bb(us, KNIGHT);
        while (knights) {
            Square from = pop_lsb(knights);
            U64 att = Bitboards::KnightAttacks[from] & ~our_pieces;
            U64 caps = att & their_pieces;
            U64 quiets = att & empty;

            while (caps) {
                list.add(Move(from, pop_lsb(caps), FLAG_CAPTURE));
            }
            if (!only_captures) {
                while (quiets) {
                    list.add(Move(from, pop_lsb(quiets), FLAG_QUIET));
                }
            }
        }

        // Bishops
        U64 bishops = board.pieces_bb(us, BISHOP);
        while (bishops) {
            Square from = pop_lsb(bishops);
            U64 att = Bitboards::get_bishop_attacks(from, occ) & ~our_pieces;
            U64 caps = att & their_pieces;
            U64 quiets = att & empty;

            while (caps) {
                list.add(Move(from, pop_lsb(caps), FLAG_CAPTURE));
            }
            if (!only_captures) {
                while (quiets) {
                    list.add(Move(from, pop_lsb(quiets), FLAG_QUIET));
                }
            }
        }

        // Rooks
        U64 rooks = board.pieces_bb(us, ROOK);
        while (rooks) {
            Square from = pop_lsb(rooks);
            U64 att = Bitboards::get_rook_attacks(from, occ) & ~our_pieces;
            U64 caps = att & their_pieces;
            U64 quiets = att & empty;

            while (caps) {
                list.add(Move(from, pop_lsb(caps), FLAG_CAPTURE));
            }
            if (!only_captures) {
                while (quiets) {
                    list.add(Move(from, pop_lsb(quiets), FLAG_QUIET));
                }
            }
        }

        // Queens
        U64 queens = board.pieces_bb(us, QUEEN);
        while (queens) {
            Square from = pop_lsb(queens);
            U64 att = Bitboards::get_queen_attacks(from, occ) & ~our_pieces;
            U64 caps = att & their_pieces;
            U64 quiets = att & empty;

            while (caps) {
                list.add(Move(from, pop_lsb(caps), FLAG_CAPTURE));
            }
            if (!only_captures) {
                while (quiets) {
                    list.add(Move(from, pop_lsb(quiets), FLAG_QUIET));
                }
            }
        }

        // King
        Square ksq = board.king_square(us);
        U64 katt = Bitboards::KingAttacks[ksq] & ~our_pieces;
        U64 kcaps = katt & their_pieces;
        U64 kquiets = katt & empty;

        while (kcaps) {
            list.add(Move(ksq, pop_lsb(kcaps), FLAG_CAPTURE));
        }
        if (!only_captures) {
            while (kquiets) {
                list.add(Move(ksq, pop_lsb(kquiets), FLAG_QUIET));
            }

            // Castling
            int cr = board.castling();
            if (us == WHITE) {
                // White Kingside: E1 to G1 (F1, G1 empty, not attacked)
                if ((cr & WHITE_OO) && !(occ & (1ULL << F1 | 1ULL << G1))) {
                    if (!board.is_square_attacked(E1, BLACK) &&
                        !board.is_square_attacked(F1, BLACK) &&
                        !board.is_square_attacked(G1, BLACK)) {
                        list.add(Move(E1, G1, FLAG_OO));
                    }
                }
                // White Queenside: E1 to C1 (D1, C1, B1 empty; E1, D1, C1 not attacked)
                if ((cr & WHITE_OOO) && !(occ & (1ULL << D1 | 1ULL << C1 | 1ULL << B1))) {
                    if (!board.is_square_attacked(E1, BLACK) &&
                        !board.is_square_attacked(D1, BLACK) &&
                        !board.is_square_attacked(C1, BLACK)) {
                        list.add(Move(E1, C1, FLAG_OOO));
                    }
                }
            } else {
                // Black Kingside: E8 to G8 (F8, G8 empty, not attacked)
                if ((cr & BLACK_OO) && !(occ & (1ULL << F8 | 1ULL << G8))) {
                    if (!board.is_square_attacked(E8, WHITE) &&
                        !board.is_square_attacked(F8, WHITE) &&
                        !board.is_square_attacked(G8, WHITE)) {
                        list.add(Move(E8, G8, FLAG_OO));
                    }
                }
                // Black Queenside: E8 to C8 (D8, C8, B8 empty; E8, D8, C8 not attacked)
                if ((cr & BLACK_OOO) && !(occ & (1ULL << D8 | 1ULL << C8 | 1ULL << B8))) {
                    if (!board.is_square_attacked(E8, WHITE) &&
                        !board.is_square_attacked(D8, WHITE) &&
                        !board.is_square_attacked(C8, WHITE)) {
                        list.add(Move(E8, C8, FLAG_OOO));
                    }
                }
            }
        }
    }

    void generate_legal_moves(Board& board, MoveList& list) {
        MoveList pseudo;
        generate_pseudo_legal_moves(board, pseudo, false);
        list.clear();
        for (int i = 0; i < pseudo.count; ++i) {
            if (board.make_move(pseudo.moves[i])) {
                board.unmake_move(pseudo.moves[i]);
                list.add(pseudo.moves[i]);
            }
        }
    }

    void generate_legal_captures(Board& board, MoveList& list) {
        MoveList pseudo;
        generate_pseudo_legal_moves(board, pseudo, true);
        list.clear();
        for (int i = 0; i < pseudo.count; ++i) {
            if (board.make_move(pseudo.moves[i])) {
                board.unmake_move(pseudo.moves[i]);
                list.add(pseudo.moves[i]);
            }
        }
    }
}
