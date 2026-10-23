#ifndef BOARD_H
#define BOARD_H

#include "types.h"
#include "bitboard.h"
#include "zobrist.h"
#include <string>
#include <vector>

const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

struct UndoInfo {
    int castling_rights;
    Square ep_square;
    int halfmove_clock;
    Piece captured;
    U64 zobrist_key;
};

class Board {
public:
    Board();

    void set_fen(const std::string& fen);
    std::string get_fen() const;
    void print() const;

    bool make_move(const Move& m);
    void unmake_move(const Move& m);
    void make_null_move();
    void unmake_null_move();

    bool is_square_attacked(Square sq, Color attacker) const;
    bool in_check() const {
        return is_square_attacked(king_square(side_to_move), ~side_to_move);
    }

    Square king_square(Color c) const {
        return lsb(pieces[make_piece(c, KING)]);
    }

    bool is_repetition(int count = 2) const;
    bool is_draw_by_50_moves() const { return halfmove_clock >= 100; }

    U64 compute_zobrist() const;

    // Piece bitboard accessors
    U64 pieces_bb(Piece p) const { return pieces[p]; }
    U64 pieces_bb(Color c, PieceType pt) const { return pieces[make_piece(c, pt)]; }
    U64 color_bb(Color c) const { return colors[c]; }
    U64 occupied_bb() const { return colors[WHITE] | colors[BLACK]; }
    Piece get_piece_at(Square sq) const { return piece_on[sq]; }

    Color turn() const { return side_to_move; }
    int castling() const { return castling_rights; }
    Square ep() const { return ep_square; }
    int half_moves() const { return halfmove_clock; }
    int full_moves() const { return fullmove_number; }
    U64 hash() const { return zobrist_key; }

private:
    U64 pieces[PIECE_NB];
    U64 colors[COLOR_NB];
    Piece piece_on[64];

    Color side_to_move;
    int castling_rights;
    Square ep_square;
    int halfmove_clock;
    int fullmove_number;
    U64 zobrist_key;

    std::vector<UndoInfo> history;
    std::vector<U64> repetition_history;

    void clear();
    void put_piece(Square sq, Piece p);
    void remove_piece(Square sq);
    void move_piece(Square from, Square to);
};

#endif // BOARD_H
