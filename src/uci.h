#ifndef UCI_H
#define UCI_H

#include "board.h"
#include "search.h"
#include <string>

class UCI {
public:
    UCI();
    void loop();
    void run_benchmark();

private:
    Board board;
    Searcher searcher;
    SearchLimits limits;
    bool own_book;

    void parse_position(std::istringstream& ss);
    void parse_go(std::istringstream& ss);
    void parse_setoption(std::istringstream& ss);
    void run_perft(int depth);
    U64 perft_recursive(Board& b, int depth);
};

#endif // UCI_H
