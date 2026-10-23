#ifndef BOOK_H
#define BOOK_H

#include "types.h"
#include <string>
#include <vector>

struct BookEntry {
    std::string fen_prefix;
    std::string move_uci;
    int weight;
    std::string opening_name;
    std::string eco;
};

namespace Book {
    void init();
    Move get_book_move(const std::string& fen, std::string* out_name = nullptr, std::string* out_eco = nullptr);
    std::string identify_opening(const std::string& fen);
}

#endif // BOOK_H
