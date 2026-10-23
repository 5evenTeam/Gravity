#include "book.h"
#include <map>
#include <random>

namespace Book {

static std::vector<BookEntry> entries;
static std::map<std::string, std::vector<BookEntry>> book_map;

static void add(const std::string& fen_sub, const std::string& move, int weight, const std::string& name, const std::string& eco) {
    BookEntry entry{fen_sub, move, weight, name, eco};
    entries.push_back(entry);
    book_map[fen_sub].push_back(entry);
}

void init() {
    book_map.clear();
    entries.clear();

    // Startpos
    add("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w", "e2e4", 45, "King's Pawn Opening", "B00");
    add("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w", "d2d4", 40, "Queen's Pawn Opening", "A40");
    add("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w", "c2c4", 10, "English Opening", "A10");
    add("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w", "g1f3", 5, "Reti Opening", "A04");

    // After 1. e4
    add("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b", "c7c5", 40, "Sicilian Defense", "B20");
    add("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b", "e7e5", 35, "Open Game", "C20");
    add("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b", "e7e6", 12, "French Defense", "C00");
    add("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b", "c7c6", 8, "Caro-Kann Defense", "B10");
    add("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b", "d7d5", 3, "Scandinavian Defense", "B01");
    add("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b", "g8f6", 2, "Alekhine Defense", "B02");

    // 1. e4 e5
    add("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w", "g1f3", 70, "King's Knight Opening", "C40");
    add("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w", "b1c3", 15, "Vienna Game", "C25");
    add("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w", "f2f4", 15, "King's Gambit", "C30");

    // 1. e4 e5 2. Nf3
    add("rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b", "b8c6", 75, "King's Knight: Normal Defense", "C40");
    add("rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b", "g8f6", 20, "Petrov's Defense", "C42");
    add("rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b", "d7d6", 5, "Philidor Defense", "C41");

    // 1. e4 e5 2. Nf3 Nc6
    add("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w", "f1b5", 50, "Ruy Lopez", "C60");
    add("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w", "f1c4", 35, "Italian Game", "C50");
    add("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w", "d2d4", 12, "Scotch Game", "C45");
    add("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w", "c2c3", 3, "Ponziani Opening", "C44");

    // Ruy Lopez: 3... a6
    add("r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b", "a7a6", 70, "Ruy Lopez: Morphy Defense", "C70");
    add("r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b", "g8f6", 30, "Ruy Lopez: Berlin Defense", "C65");
    add("r1bqkbnr/1ppp1ppp/p1n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w", "b5a4", 95, "Ruy Lopez: Morphy Defense", "C70");

    // Italian Game: 3... Bc5 / Nf6
    add("r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b", "f8c5", 60, "Giuoco Piano", "C50");
    add("r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b", "g8f6", 40, "Two Knights Defense", "C55");

    // Sicilian: 1. e4 c5
    add("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w", "g1f3", 75, "Sicilian: Open Setup", "B27");
    add("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w", "b1c3", 15, "Sicilian: Closed", "B23");
    add("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w", "c2c3", 10, "Sicilian: Alapin", "B22");

    add("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b", "d7d6", 45, "Sicilian Defense", "B50");
    add("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b", "b8c6", 30, "Sicilian: Old Sicilian", "B30");
    add("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b", "e7e6", 25, "Sicilian: French Variation", "B40");

    // French Defense: 1. e4 e6 2. d4 d5
    add("rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w", "d2d4", 90, "French Defense", "C00");
    add("rnbqkbnr/pppp1ppp/4p3/8/3PP3/8/PPP2PPP/RNBQKBNR b", "d7d5", 90, "French Defense", "C00");
    add("rnbqkbnr/ppp2ppp/4p3/3p4/3PP3/8/PPP2PPP/RNBQKBNR w", "b1c3", 45, "French: Paulsen", "C10");
    add("rnbqkbnr/ppp2ppp/4p3/3p4/3PP3/8/PPP2PPP/RNBQKBNR w", "e4e5", 35, "French: Advance Variation", "C02");
    add("rnbqkbnr/ppp2ppp/4p3/3p4/3PP3/8/PPP2PPP/RNBQKBNR w", "b1d2", 20, "French: Tarrasch", "C03");

    // Caro-Kann: 1. e4 c6 2. d4 d5
    add("rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w", "d2d4", 90, "Caro-Kann Defense", "B12");
    add("rnbqkbnr/pp1ppppp/2p5/8/3PP3/8/PPP2PPP/RNBQKBNR b", "d7d5", 90, "Caro-Kann Defense", "B12");
    add("rnbqkbnr/pp2pppp/2p5/3p4/3PP3/8/PPP2PPP/RNBQKBNR w", "b1c3", 50, "Caro-Kann: Main Line", "B15");
    add("rnbqkbnr/pp2pppp/2p5/3p4/3PP3/8/PPP2PPP/RNBQKBNR w", "e4e5", 40, "Caro-Kann: Advance Variation", "B12");

    // 1. d4
    add("rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b", "g8f6", 50, "Indian Defense", "A45");
    add("rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b", "d7d5", 40, "Closed Game", "D00");
    add("rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b", "f7f5", 10, "Dutch Defense", "A80");

    // 1. d4 d5
    add("rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w", "c2c4", 75, "Queen's Gambit", "D06");
    add("rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w", "c1f4", 15, "London System", "D00");
    add("rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w", "g1f3", 10, "Colle System", "D04");

    // Queen's Gambit: 1. d4 d5 2. c4
    add("rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b", "e7e6", 50, "Queen's Gambit Declined", "D30");
    add("rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b", "c7c6", 35, "Slav Defense", "D10");
    add("rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b", "d5c4", 15, "Queen's Gambit Accepted", "D20");

    // 1. d4 Nf6
    add("rnbqkb1r/pppppppp/5n2/8/3P4/8/PPP1PPPP/RNBQKBNR w", "c2c4", 80, "Indian Defense", "E00");
    add("rnbqkb1r/pppppppp/5n2/8/3P4/8/PPP1PPPP/RNBQKBNR w", "c1g5", 10, "Trompowsky Attack", "A45");
    add("rnbqkb1r/pppppppp/5n2/8/3P4/8/PPP1PPPP/RNBQKBNR w", "g1f3", 10, "King's Indian Setup", "A46");

    // 1. d4 Nf6 2. c4
    add("rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b", "g7g6", 45, "King's Indian / Grunfeld", "E60");
    add("rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b", "e7e6", 40, "Nimzo / Queen's Indian / Catalan", "E00");
    add("rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b", "c7c5", 15, "Benoni Defense", "A56");

    // King's Indian & Grunfeld: 1. d4 Nf6 2. c4 g6 3. Nc3
    add("rnbqkb1r/pppppp1p/5np1/8/2PP4/8/PP2PPPP/RNBQKBNR w", "b1c3", 90, "King's Indian Defense", "E60");
    add("rnbqkb1r/pppppp1p/5np1/8/2PP4/2N5/PP2PPPP/R1BQKBNR b", "f8g7", 60, "King's Indian Defense", "E60");
    add("rnbqkb1r/pppppp1p/5np1/8/2PP4/2N5/PP2PPPP/R1BQKBNR b", "d7d5", 40, "Grunfeld Defense", "D80");

    // Nimzo-Indian / Bogo-Indian / Catalan
    add("rnbqkb1r/pppp1ppp/4pn2/8/2PP4/8/PP2PPPP/RNBQKBNR w", "b1c3", 50, "Nimzo-Indian Defense Setup", "E20");
    add("rnbqkb1r/pppp1ppp/4pn2/8/2PP4/8/PP2PPPP/RNBQKBNR w", "g1f3", 35, "Queen's Indian Setup", "E12");
    add("rnbqkb1r/pppp1ppp/4pn2/8/2PP4/8/PP2PPPP/RNBQKBNR w", "g2g3", 15, "Catalan Opening", "E01");
}

Move get_book_move(const std::string& fen, std::string* out_name, std::string* out_eco) {
    if (book_map.empty()) init();

    // Extract first 2-3 tokens: "piece_placement side"
    size_t space1 = fen.find(' ');
    if (space1 == std::string::npos) return NULL_MOVE;
    size_t space2 = fen.find(' ', space1 + 1);
    std::string key = (space2 == std::string::npos) ? fen : fen.substr(0, space2);

    auto it = book_map.find(key);
    if (it == book_map.end() || it->second.empty()) return NULL_MOVE;

    const auto& choices = it->second;
    int total_weight = 0;
    for (const auto& e : choices) total_weight += e.weight;

    static std::mt19937 rng(1337);
    int r = rng() % total_weight;
    int current = 0;

    for (const auto& e : choices) {
        current += e.weight;
        if (r < current) {
            if (out_name) *out_name = e.opening_name;
            if (out_eco) *out_eco = e.eco;

            Square from = str_to_square(e.move_uci.substr(0, 2));
            Square to = str_to_square(e.move_uci.substr(2, 2));
            int flag = FLAG_QUIET;
            if (e.move_uci.length() > 4) {
                char p = e.move_uci[4];
                if (p == 'q') flag = FLAG_PROMO_Q;
                else if (p == 'r') flag = FLAG_PROMO_R;
                else if (p == 'b') flag = FLAG_PROMO_B;
                else if (p == 'n') flag = FLAG_PROMO_N;
            }
            return Move(from, to, flag);
        }
    }

    return NULL_MOVE;
}

std::string identify_opening(const std::string& fen) {
    if (book_map.empty()) init();
    size_t space1 = fen.find(' ');
    if (space1 == std::string::npos) return "";
    size_t space2 = fen.find(' ', space1 + 1);
    std::string key = (space2 == std::string::npos) ? fen : fen.substr(0, space2);

    auto it = book_map.find(key);
    if (it != book_map.end() && !it->second.empty()) {
        return it->second[0].eco + " " + it->second[0].opening_name;
    }
    return "";
}

}
