#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "types.h"
#include <atomic>
#include <chrono>
#include <vector>
#include <thread>

struct MoveList;

struct SearchLimits {
    int time_ms = -1;
    int depth = 64;
    int max_nodes = -1;
    bool infinite = false;
    int elo = 4000;
    bool limit_strength = false;
    int num_threads = 4;
};

struct SearchInfo {
    int depth = 0;
    int seldepth = 0;
    int score = 0;
    std::atomic<U64> nodes{0};
    std::chrono::steady_clock::time_point start_time;
    Move best_move;
    std::vector<Move> pv;
    std::atomic<bool> stopped{false};
};

class Searcher {
public:
    Searcher();
    ~Searcher();

    void reset();
    void set_limits(const SearchLimits& limits);
    void set_threads(int threads);
    void stop();
    Move start_search(Board& board, const SearchLimits& limits);

    SearchInfo& get_info() { return info; }

private:
    SearchLimits limits;
    SearchInfo info;

    struct WorkerData {
        Move killer_moves[128][2];
        int history_moves[COLOR_NB][64][64];

        void clear() {
            for (int i = 0; i < 128; ++i) {
                killer_moves[i][0] = NULL_MOVE;
                killer_moves[i][1] = NULL_MOVE;
            }
            for (int c = 0; c < COLOR_NB; ++c) {
                for (int i = 0; i < 64; ++i) {
                    for (int j = 0; j < 64; ++j) {
                        history_moves[c][i][j] = 0;
                    }
                }
            }
        }
    };

    WorkerData main_worker;
    std::vector<std::thread> helper_threads;

    bool should_stop();
    int quiescence(Board& board, int alpha, int beta, int ply, WorkerData& wd);
    int negamax(Board& board, int depth, int alpha, int beta, int ply, bool is_pv, WorkerData& wd);
    void score_moves(Board& board, MoveList& list, Move hash_move, int ply, WorkerData& wd);
    void pick_move(MoveList& list, int current_index);
    void helper_search(Board board, int start_depth, int thread_id);
};

#endif // SEARCH_H
