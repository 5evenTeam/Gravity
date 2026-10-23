#ifndef TT_H
#define TT_H

#include "types.h"
#include <vector>

enum TTFlag : uint8_t {
    TT_EXACT = 0,
    TT_LOWERBOUND = 1, // Beta cutoff
    TT_UPPERBOUND = 2  // Alpha fail-low
};

struct TTEntry {
    U64 key;
    int16_t score;
    uint8_t depth;
    uint8_t flag;
    uint16_t move_data;
    uint8_t age;
};

class TranspositionTable {
public:
    TranspositionTable(size_t mb = 64);
    ~TranspositionTable();

    void resize(size_t mb);
    void clear();
    void new_search();

    bool probe(U64 key, int depth, int alpha, int beta, int ply, int& score, Move& best_move);
    void store(U64 key, int depth, int flag, int score, Move best_move, int ply);
    Move get_best_move(U64 key);

    size_t get_size_mb() const { return size_mb; }

private:
    std::vector<TTEntry> table;
    size_t num_entries;
    size_t size_mb;
    uint8_t current_age;
};

extern TranspositionTable g_tt;

#endif // TT_H
