#include "tt.h"
#include <cstring>

TranspositionTable g_tt(64);

TranspositionTable::TranspositionTable(size_t mb) : num_entries(0), size_mb(0), current_age(0) {
    resize(mb);
}

TranspositionTable::~TranspositionTable() {}

void TranspositionTable::resize(size_t mb) {
    size_mb = mb;
    num_entries = (mb * 1024 * 1024) / sizeof(TTEntry);
    table.resize(num_entries);
    clear();
}

void TranspositionTable::clear() {
    std::fill(table.begin(), table.end(), TTEntry{0, 0, 0, 0, 0, 0});
    current_age = 0;
}

void TranspositionTable::new_search() {
    current_age++;
}

bool TranspositionTable::probe(U64 key, int depth, int alpha, int beta, int ply, int& score, Move& best_move) {
    size_t index = key % num_entries;
    const TTEntry& entry = table[index];

    if (entry.key == key) {
        best_move = Move(entry.move_data);

        if (entry.depth >= depth) {
            int entry_score = entry.score;
            // Adjust mate scores relative to current ply
            if (entry_score > MATE_SCORE - 100) entry_score -= ply;
            else if (entry_score < -MATE_SCORE + 100) entry_score += ply;

            if (entry.flag == TT_EXACT) {
                score = entry_score;
                return true;
            }
            if (entry.flag == TT_LOWERBOUND && entry_score >= beta) {
                score = entry_score;
                return true;
            }
            if (entry.flag == TT_UPPERBOUND && entry_score <= alpha) {
                score = entry_score;
                return true;
            }
        }
    }
    return false;
}

Move TranspositionTable::get_best_move(U64 key) {
    size_t index = key % num_entries;
    const TTEntry& entry = table[index];
    if (entry.key == key) {
        return Move(entry.move_data);
    }
    return NULL_MOVE;
}

void TranspositionTable::store(U64 key, int depth, int flag, int score, Move best_move, int ply) {
    size_t index = key % num_entries;
    TTEntry& entry = table[index];

    // Adjust mate scores to be independent of ply
    if (score > MATE_SCORE - 100) score += ply;
    else if (score < -MATE_SCORE + 100) score -= ply;

    // Replacement strategy: replace if empty, different key, deeper or equal depth, or older age
    if (entry.key == 0 || entry.key != key || depth >= entry.depth || entry.age != current_age) {
        entry.key = key;
        entry.depth = static_cast<uint8_t>(depth);
        entry.flag = static_cast<uint8_t>(flag);
        entry.score = static_cast<int16_t>(score);
        if (!best_move.is_null()) {
            entry.move_data = best_move.data;
        }
        entry.age = current_age;
    }
}
