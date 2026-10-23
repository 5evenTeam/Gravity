#include "types.h"
#include "bitboard.h"
#include "zobrist.h"
#include "book.h"
#include "uci.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    Bitboards::init();
    Zobrist::init();
    Book::init();

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--bench" || arg == "bench") {
            std::cout << "Gravity benchmark..." << std::endl;
            UCI uci;
            std::istringstream ss("perft 5");
            return 0;
        } else if (arg == "--version" || arg == "-v") {
            std::cout << "Gravity 4.0" << std::endl;
            return 0;
        }
    }

    UCI uci;
    uci.loop();

    return 0;
}
