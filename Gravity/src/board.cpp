#include "board.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

static const int CastlingPerm[64] = {
    13, 15, 15, 15, 12, 15, 15, 14, // 0-7 (A1-H1): A1=15^2=13, E1=15^3=12, H1=15^1=14
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
     7, 15, 15, 15,  3, 15, 15, 11  // 56-63 (A8-H8): A8=15^8=7, E8=15^12=3, H8=15^4=11
};

Board::Board() {
    clear();
    set_fen(START_FEN);
}

void Board::clear() {
    for (int p = 0; p < PIECE_NB; ++p) pieces[p] = 0;
    colors[WHITE] = 0;
    colors[BLACK] = 0;
    for (int sq = 0; sq < 64; ++sq) piece_on[sq] = NO_PIECE;

    side_to_move = WHITE;
    castling_rights = 0;
    ep_square = NO_SQ;
    halfmove_clock = 0;
    fullmove_number = 1;
    zobrist_key = 0;

    history.clear();
    repetition_history.clear();
}

void Board::put_piece(Square sq, Piece p) {
    pieces[p] |= (1ULL << sq);
    colors[piece_color(p)] |= (1ULL << sq);
    piece_on[sq] = p;
    zobrist_key ^= Zobrist::PieceSquare[p][sq];
}

void Board::remove_piece(Square sq) {
    Piece p = piece_on[sq];
    if (p == NO_PIECE) return;
    pieces[p] &= ~(1ULL << sq);
    colors[piece_color(p)] &= ~(1ULL << sq);
    piece_on[sq] = NO_PIECE;
    zobrist_key ^= Zobrist::PieceSquare[p][sq];
}

void Board::move_piece(Square from, Square to) {
    Piece p = piece_on[from];
    remove_piece(from);
    put_piece(to, p);
}

U64 Board::compute_zobrist() const {
    U64 key = 0;
    for (int sq = 0; sq < 64; ++sq) {
        Piece p = piece_on[sq];
        if (p != NO_PIECE) {
            key ^= Zobrist::PieceSquare[p][sq];
        }
    }
    if (side_to_move == BLACK) {
        key ^= Zobrist::SideToMove;
    }
    key ^= Zobrist::Castling[castling_rights];
    if (ep_square != NO_SQ) {
        key ^= Zobrist::EnPassant[square_file(ep_square)];
    }
    return key;
}

void Board::set_fen(const std::string& fen) {
    clear();
    std::istringstream ss(fen);
    std::string piece_str, turn_str, castling_str, ep_str;
    int half_clock = 0, full_num = 1;

    ss >> piece_str >> turn_str >> castling_str >> ep_str >> half_clock >> full_num;

    int r = 7, f = 0;
    for (char c : piece_str) {
        if (c == '/') {
            r--;
            f = 0;
        } else if (c >= '1' && c <= '8') {
            f += (c - '0');
        } else {
            Square sq = make_square(f, r);
            Piece p = NO_PIECE;
            switch (c) {
                case 'P': p = W_PAWN; break;
                case 'N': p = W_KNIGHT; break;
                case 'B': p = W_BISHOP; break;
                case 'R': p = W_ROOK; break;
                case 'Q': p = W_QUEEN; break;
                case 'K': p = W_KING; break;
                case 'p': p = B_PAWN; break;
                case 'n': p = B_KNIGHT; break;
                case 'b': p = B_BISHOP; break;
                case 'r': p = B_ROOK; break;
                case 'q': p = B_QUEEN; break;
                case 'k': p = B_KING; break;
                default: break;
            }
            if (p != NO_PIECE) put_piece(sq, p);
            f++;
        }
    }

    side_to_move = (turn_str == "b") ? BLACK : WHITE;

    castling_rights = 0;
    if (castling_str.find('K') != std::string::npos) castling_rights |= WHITE_OO;
    if (castling_str.find('Q') != std::string::npos) castling_rights |= WHITE_OOO;
    if (castling_str.find('k') != std::string::npos) castling_rights |= BLACK_OO;
    if (castling_str.find('q') != std::string::npos) castling_rights |= BLACK_OOO;

    if (ep_str != "-" && ep_str.length() >= 2) {
        ep_square = str_to_square(ep_str);
    } else {
        ep_square = NO_SQ;
    }

    halfmove_clock = half_clock;
    fullmove_number = full_num > 0 ? full_num : 1;

    zobrist_key = compute_zobrist();
    repetition_history.push_back(zobrist_key);
}

std::string Board::get_fen() const {
    std::string fen = "";
    for (int r = 7; r >= 0; --r) {
        int empty = 0;
        for (int f = 0; f < 8; ++f) {
            Square sq = make_square(f, r);
            Piece p = piece_on[sq];
            if (p == NO_PIECE) {
                empty++;
            } else {
                if (empty > 0) {
                    fen += std::to_string(empty);
                    empty = 0;
                }
                const char piece_chars[] = "PNBRQKpnbrqk";
                fen += piece_chars[p];
            }
        }
        if (empty > 0) fen += std::to_string(empty);
        if (r > 0) fen += "/";
    }

    fen += (side_to_move == WHITE ? " w " : " b ");

    std::string castling_str = "";
    if (castling_rights & WHITE_OO)  castling_str += 'K';
    if (castling_rights & WHITE_OOO) castling_str += 'Q';
    if (castling_rights & BLACK_OO)  castling_str += 'k';
    if (castling_rights & BLACK_OOO) castling_str += 'q';
    if (castling_str.empty()) castling_str = "-";
    fen += castling_str + " ";

    if (ep_square != NO_SQ) {
        fen += square_to_str(ep_square);
    } else {
        fen += "-";
    }

    fen += " " + std::to_string(halfmove_clock) + " " + std::to_string(fullmove_number);
    return fen;
}

bool Board::is_square_attacked(Square sq, Color attacker) const {
    U64 occ = occupied_bb();

    // Pawn attacks: If attacker attacks sq, then sq pawn-attacked backwards has an attacker pawn
    Color defender = ~attacker;
    if (Bitboards::PawnAttacks[defender][sq] & pieces[make_piece(attacker, PAWN)]) return true;

    // Knight attacks
    if (Bitboards::KnightAttacks[sq] & pieces[make_piece(attacker, KNIGHT)]) return true;

    // King attacks
    if (Bitboards::KingAttacks[sq] & pieces[make_piece(attacker, KING)]) return true;

    // Bishop / Queen diagonals
    U64 bq = pieces[make_piece(attacker, BISHOP)] | pieces[make_piece(attacker, QUEEN)];
    if (bq && (Bitboards::get_bishop_attacks(sq, occ) & bq)) return true;

    // Rook / Queen orthogonals
    U64 rq = pieces[make_piece(attacker, ROOK)] | pieces[make_piece(attacker, QUEEN)];
    if (rq && (Bitboards::get_rook_attacks(sq, occ) & rq)) return true;

    return false;
}

bool Board::make_move(const Move& m) {
    Square from = m.from();
    Square to = m.to();
    int flags = m.flags();

    Piece moving_piece = piece_on[from];
    Piece captured_piece = piece_on[to];

    // Save undo information
    UndoInfo undo;
    undo.castling_rights = castling_rights;
    undo.ep_square = ep_square;
    undo.halfmove_clock = halfmove_clock;
    undo.captured = captured_piece;
    undo.zobrist_key = zobrist_key;
    history.push_back(undo);

    // Update halfmove clock
    if (piece_type(moving_piece) == PAWN || m.is_capture()) {
        halfmove_clock = 0;
    } else {
        halfmove_clock++;
    }

    // Hash out current castling & ep
    zobrist_key ^= Zobrist::Castling[castling_rights];
    if (ep_square != NO_SQ) {
        zobrist_key ^= Zobrist::EnPassant[square_file(ep_square)];
        ep_square = NO_SQ;
    }

    // Handle captures
    if (m.is_capture()) {
        if (m.is_en_passant()) {
            Square cap_sq = (side_to_move == WHITE) ? static_cast<Square>(to - 8) : static_cast<Square>(to + 8);
            remove_piece(cap_sq);
        } else {
            remove_piece(to);
        }
    }

    // Move the piece
    remove_piece(from);

    if (m.is_promotion()) {
        PieceType promo = m.promotion_type();
        put_piece(to, make_piece(side_to_move, promo));
    } else {
        put_piece(to, moving_piece);
    }

    // Handle castling rook moves
    if (flags == FLAG_OO) {
        if (side_to_move == WHITE) {
            remove_piece(H1);
            put_piece(F1, W_ROOK);
        } else {
            remove_piece(H8);
            put_piece(F8, B_ROOK);
        }
    } else if (flags == FLAG_OOO) {
        if (side_to_move == WHITE) {
            remove_piece(A1);
            put_piece(D1, W_ROOK);
        } else {
            remove_piece(A8);
            put_piece(D8, B_ROOK);
        }
    }

    // Double pawn push sets en passant square
    if (flags == FLAG_DOUBLE_PAWN) {
        ep_square = (side_to_move == WHITE) ? static_cast<Square>(from + 8) : static_cast<Square>(from - 8);
        zobrist_key ^= Zobrist::EnPassant[square_file(ep_square)];
    }

    // Update castling rights
    castling_rights &= CastlingPerm[from] & CastlingPerm[to];
    zobrist_key ^= Zobrist::Castling[castling_rights];

    // Toggle side to move
    side_to_move = ~side_to_move;
    zobrist_key ^= Zobrist::SideToMove;

    if (side_to_move == WHITE) fullmove_number++;

    // Check legality: Did the mover leave their own king in check?
    if (is_square_attacked(king_square(~side_to_move), side_to_move)) {
        unmake_move(m);
        return false;
    }

    repetition_history.push_back(zobrist_key);
    return true;
}

void Board::unmake_move(const Move& m) {
    if (history.empty()) return;

    if (!repetition_history.empty()) repetition_history.pop_back();

    UndoInfo undo = history.back();
    history.pop_back();

    side_to_move = ~side_to_move;
    if (side_to_move == BLACK) fullmove_number--;

    Square from = m.from();
    Square to = m.to();
    int flags = m.flags();

    // Revert piece move or promotion
    Piece moved_piece = piece_on[to];
    remove_piece(to);
    if (m.is_promotion()) {
        put_piece(from, make_piece(side_to_move, PAWN));
    } else {
        put_piece(from, moved_piece);
    }

    // Restore captured piece
    if (m.is_capture()) {
        if (m.is_en_passant()) {
            Square cap_sq = (side_to_move == WHITE) ? static_cast<Square>(to - 8) : static_cast<Square>(to + 8);
            put_piece(cap_sq, make_piece(~side_to_move, PAWN));
        } else {
            put_piece(to, undo.captured);
        }
    }

    // Revert castling rooks
    if (flags == FLAG_OO) {
        if (side_to_move == WHITE) {
            remove_piece(F1);
            put_piece(H1, W_ROOK);
        } else {
            remove_piece(F8);
            put_piece(H8, B_ROOK);
        }
    } else if (flags == FLAG_OOO) {
        if (side_to_move == WHITE) {
            remove_piece(D1);
            put_piece(A1, W_ROOK);
        } else {
            remove_piece(D8);
            put_piece(A8, B_ROOK);
        }
    }

    castling_rights = undo.castling_rights;
    ep_square = undo.ep_square;
    halfmove_clock = undo.halfmove_clock;
    zobrist_key = undo.zobrist_key;
}

void Board::make_null_move() {
    UndoInfo undo;
    undo.castling_rights = castling_rights;
    undo.ep_square = ep_square;
    undo.halfmove_clock = halfmove_clock;
    undo.captured = NO_PIECE;
    undo.zobrist_key = zobrist_key;
    history.push_back(undo);

    if (ep_square != NO_SQ) {
        zobrist_key ^= Zobrist::EnPassant[square_file(ep_square)];
        ep_square = NO_SQ;
    }

    side_to_move = ~side_to_move;
    zobrist_key ^= Zobrist::SideToMove;
    repetition_history.push_back(zobrist_key);
}

void Board::unmake_null_move() {
    if (history.empty()) return;
    if (!repetition_history.empty()) repetition_history.pop_back();

    UndoInfo undo = history.back();
    history.pop_back();

    side_to_move = ~side_to_move;
    castling_rights = undo.castling_rights;
    ep_square = undo.ep_square;
    halfmove_clock = undo.halfmove_clock;
    zobrist_key = undo.zobrist_key;
}

bool Board::is_repetition(int count) const {
    int matches = 0;
    int limit = std::max(0, static_cast<int>(repetition_history.size()) - 1 - halfmove_clock);
    for (int i = static_cast<int>(repetition_history.size()) - 1; i >= limit; i -= 2) {
        if (repetition_history[i] == zobrist_key) {
            matches++;
            if (matches >= count) return true;
        }
    }
    return false;
}

void Board::print() const {
    std::cout << "\n  +---+---+---+---+---+---+---+---+\n";
    for (int r = 7; r >= 0; --r) {
        std::cout << (r + 1) << " |";
        for (int f = 0; f < 8; ++f) {
            Piece p = piece_on[make_square(f, r)];
            char ch = '.';
            if (p != NO_PIECE) {
                const char* chars = "PNBRQKpnbrqk";
                ch = chars[p];
            }
            std::cout << " " << ch << " |";
        }
        std::cout << "\n  +---+---+---+---+---+---+---+---+\n";
    }
    std::cout << "    a   b   c   d   e   f   g   h\n\n";
    std::cout << "FEN: " << get_fen() << "\n";
    std::cout << "Turn: " << (side_to_move == WHITE ? "White" : "Black")
              << " | Castling: " << castling_rights
              << " | EP: " << (ep_square != NO_SQ ? square_to_str(ep_square) : "-")
              << " | Check: " << (in_check() ? "YES" : "NO") << "\n\n";
}
