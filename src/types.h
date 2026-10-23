#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <string>
#include <iostream>
#include <vector>

using U64 = uint64_t;

enum Color : int {
    WHITE = 0,
    BLACK = 1,
    COLOR_NB = 2
};

inline Color operator~(Color c) {
    return static_cast<Color>(c ^ 1);
}

enum PieceType : int {
    PAWN = 0,
    KNIGHT = 1,
    BISHOP = 2,
    ROOK = 3,
    QUEEN = 4,
    KING = 5,
    ALL_PIECES = 6,
    PIECE_TYPE_NB = 7
};

enum Piece : int {
    W_PAWN = 0, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = 6, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    NO_PIECE = 12,
    PIECE_NB = 12
};

inline Piece make_piece(Color c, PieceType pt) {
    return static_cast<Piece>((c == WHITE ? 0 : 6) + pt);
}

inline PieceType piece_type(Piece p) {
    return static_cast<PieceType>(p % 6);
}

inline Color piece_color(Piece p) {
    return static_cast<Color>(p / 6);
}

enum Square : int {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQ = 64
};

inline Square make_square(int file, int rank) {
    return static_cast<Square>(rank * 8 + file);
}

inline int square_file(Square sq) {
    return sq & 7;
}

inline int square_rank(Square sq) {
    return sq >> 3;
}

enum CastlingRights : int {
    NO_CASTLING = 0,
    WHITE_OO = 1,
    WHITE_OOO = 2,
    BLACK_OO = 4,
    BLACK_OOO = 8,
    WHITE_CASTLING = WHITE_OO | WHITE_OOO,
    BLACK_CASTLING = BLACK_OO | BLACK_OOO,
    ALL_CASTLING = WHITE_CASTLING | BLACK_CASTLING
};

enum MoveFlag : int {
    FLAG_QUIET = 0,
    FLAG_DOUBLE_PAWN = 1,
    FLAG_OO = 2,
    FLAG_OOO = 3,
    FLAG_CAPTURE = 4,
    FLAG_EN_PASSANT = 5,
    FLAG_PROMO_N = 8,
    FLAG_PROMO_B = 9,
    FLAG_PROMO_R = 10,
    FLAG_PROMO_Q = 11,
    FLAG_PROMO_CAP_N = 12,
    FLAG_PROMO_CAP_B = 13,
    FLAG_PROMO_CAP_R = 14,
    FLAG_PROMO_CAP_Q = 15
};

struct Move {
    uint16_t data;

    Move() : data(0) {}
    Move(uint16_t d) : data(d) {}
    Move(Square from, Square to, int flags = FLAG_QUIET) {
        data = static_cast<uint16_t>((from & 0x3F) | ((to & 0x3F) << 6) | ((flags & 0xF) << 12));
    }

    Square from() const { return static_cast<Square>(data & 0x3F); }
    Square to() const { return static_cast<Square>((data >> 6) & 0x3F); }
    int flags() const { return (data >> 12) & 0xF; }

    bool is_capture() const { return (flags() & 4) != 0; }
    bool is_promotion() const { return (flags() & 8) != 0; }
    bool is_en_passant() const { return flags() == FLAG_EN_PASSANT; }
    bool is_castling() const { return flags() == FLAG_OO || flags() == FLAG_OOO; }

    PieceType promotion_type() const {
        if (!is_promotion()) return PAWN;
        switch (flags() & 3) {
            case 0: return KNIGHT;
            case 1: return BISHOP;
            case 2: return ROOK;
            default: return QUEEN;
        }
    }

    bool operator==(const Move& other) const { return data == other.data; }
    bool operator!=(const Move& other) const { return data != other.data; }
    bool is_null() const { return data == 0; }
};

inline const Move NULL_MOVE = Move();

// Helper to convert Square to UCI string like "e2", "e4"
inline std::string square_to_str(Square sq) {
    if (sq < A1 || sq > H8) return "-";
    char f = 'a' + square_file(sq);
    char r = '1' + square_rank(sq);
    return std::string{f, r};
}

inline Square str_to_square(const std::string& str) {
    if (str.length() < 2) return NO_SQ;
    int f = str[0] - 'a';
    int r = str[1] - '1';
    if (f < 0 || f > 7 || r < 0 || r > 7) return NO_SQ;
    return make_square(f, r);
}

inline std::string move_to_uci(const Move& m) {
    if (m.is_null()) return "0000";
    std::string res = square_to_str(m.from()) + square_to_str(m.to());
    if (m.is_promotion()) {
        switch (m.promotion_type()) {
            case KNIGHT: res += 'n'; break;
            case BISHOP: res += 'b'; break;
            case ROOK:   res += 'r'; break;
            case QUEEN:  res += 'q'; break;
            default: break;
        }
    }
    return res;
}

const int INF = 30000;
const int MATE_SCORE = 29000;

#endif // TYPES_H
