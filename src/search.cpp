#include "search.h"
#include "movegen.h"
#include "evaluate.h"
#include "tt.h"
#include <iostream>
#include <algorithm>
#include <random>

static const int MVV_LVA[6][6] = {
    // Victim: Pawn (0), Knight (1), Bishop (2), Rook (3), Queen (4), King (5)
    // Attacker: P, N, B, R, Q, K
    { 105, 104, 103, 102, 101, 100 }, // Pawn
    { 205, 204, 203, 202, 201, 200 }, // Knight
    { 305, 304, 303, 302, 301, 300 }, // Bishop
    { 405, 404, 403, 402, 401, 400 }, // Rook
    { 505, 504, 503, 502, 501, 500 }, // Queen
    { 605, 604, 603, 602, 601, 600 }  // King
};

Searcher::Searcher() {
    main_worker.clear();
}

Searcher::~Searcher() {
    stop();
}

void Searcher::reset() {
    main_worker.clear();
    info.stopped = false;
    info.nodes = 0;
}

void Searcher::set_limits(const SearchLimits& l) {
    limits = l;
}

void Searcher::set_threads(int threads) {
    limits.num_threads = std::max(1, std::min(threads, 64));
}

void Searcher::stop() {
    info.stopped = true;
    for (auto& th : helper_threads) {
        if (th.joinable()) th.join();
    }
    helper_threads.clear();
}

bool Searcher::should_stop() {
    if (info.stopped) return true;
    const U64 nodes = info.nodes.load(std::memory_order_relaxed);
    // A node-limited UCI search must not wait for the periodic clock check;
    // otherwise `go nodes 1` could search thousands of nodes.
    if (limits.max_nodes > 0 && nodes >= static_cast<U64>(limits.max_nodes)) {
        info.stopped = true;
        return true;
    }
    if ((nodes & 2047) == 0) {
        if (limits.time_ms > 0) {
            auto now = std::chrono::steady_clock::now();
            long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.start_time).count();
            if (elapsed >= limits.time_ms) {
                info.stopped = true;
                return true;
            }
        }
    }
    return false;
}

void Searcher::score_moves(Board& board, MoveList& list, Move hash_move, int ply, WorkerData& wd) {
    Color us = board.turn();
    for (int i = 0; i < list.count; ++i) {
        Move m = list.moves[i];
        if (m == hash_move) {
            list.scores[i] = 2000000;
        } else if (m.is_capture()) {
            Piece victim = board.get_piece_at(m.to());
            Piece attacker = board.get_piece_at(m.from());
            PieceType v_type = (victim != NO_PIECE) ? piece_type(victim) : PAWN;
            PieceType a_type = piece_type(attacker);
            list.scores[i] = 1000000 + MVV_LVA[v_type][a_type];
        } else if (ply < 128 && m == wd.killer_moves[ply][0]) {
            list.scores[i] = 900000;
        } else if (ply < 128 && m == wd.killer_moves[ply][1]) {
            list.scores[i] = 800000;
        } else {
            list.scores[i] = wd.history_moves[us][m.from()][m.to()];
        }
    }
}

void Searcher::pick_move(MoveList& list, int current_index) {
    int best_idx = current_index;
    for (int i = current_index + 1; i < list.count; ++i) {
        if (list.scores[i] > list.scores[best_idx]) {
            best_idx = i;
        }
    }
    if (best_idx != current_index) {
        std::swap(list.moves[current_index], list.moves[best_idx]);
        std::swap(list.scores[current_index], list.scores[best_idx]);
    }
}

int Searcher::quiescence(Board& board, int alpha, int beta, int ply, WorkerData& wd) {
    if (should_stop()) return 0;
    info.nodes.fetch_add(1, std::memory_order_relaxed);

    if (ply > info.seldepth) info.seldepth = ply;

    // A static evaluation while the side to move is in check is not a valid
    // quiescent position. Search every legal evasion (including quiet king
    // moves and interpositions) instead of only captures.
    bool in_check = board.in_check();
    int stand_pat = Eval::evaluate(board);
    if (!in_check) {
        if (stand_pat >= beta) return beta;
        if (stand_pat > alpha) alpha = stand_pat;
    }

    if (ply >= 127) return stand_pat;

    MoveList list;
    if (in_check) {
        MoveGen::generate_legal_moves(board, list);
    } else {
        MoveGen::generate_legal_captures(board, list);
    }
    if (in_check && list.count == 0) {
        return -MATE_SCORE + ply;
    }
    score_moves(board, list, NULL_MOVE, ply, wd);

    for (int i = 0; i < list.count; ++i) {
        pick_move(list, i);
        Move m = list.moves[i];

        // Delta pruning: If static eval + Queen value is still below alpha, skip
        if (!in_check && stand_pat + 1025 < alpha && !m.is_promotion()) {
            continue;
        }

        board.make_move(m);
        int score = -quiescence(board, -beta, -alpha, ply + 1, wd);
        board.unmake_move(m);

        if (info.stopped) return 0;

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

int Searcher::negamax(Board& board, int depth, int alpha, int beta, int ply, bool is_pv, WorkerData& wd) {
    if (should_stop()) return 0;

    // Check for draw by 50-move rule or 3-fold repetition
    if (ply > 0 && (board.is_draw_by_50_moves() || board.is_repetition())) {
        return 0;
    }

    bool in_check = board.in_check();

    // Check extension
    if (in_check) depth++;

    if (depth <= 0) {
        return quiescence(board, alpha, beta, ply, wd);
    }

    info.nodes.fetch_add(1, std::memory_order_relaxed);
    if (ply > info.seldepth) info.seldepth = ply;

    // Transposition Table probe
    Move hash_move = NULL_MOVE;
    int tt_score;
    if (!is_pv && g_tt.probe(board.hash(), depth, alpha, beta, ply, tt_score, hash_move)) {
        return tt_score;
    }

    int static_eval = Eval::evaluate(board);

    // Reverse Futility Pruning (Static null move pruning)
    if (!is_pv && !in_check && depth <= 3) {
        int margin = 120 * depth;
        if (static_eval - margin >= beta) {
            return static_eval - margin;
        }
    }

    // Null Move Pruning
    if (!is_pv && !in_check && depth >= 3 && static_eval >= beta) {
        U64 non_pawns = board.color_bb(board.turn()) ^ (board.pieces_bb(board.turn(), PAWN) | board.pieces_bb(board.turn(), KING));
        if (non_pawns) {
            int R = 2 + (depth >= 6 ? 1 : 0);
            board.make_null_move();
            int null_score = -negamax(board, depth - 1 - R, -beta, -beta + 1, ply + 1, false, wd);
            board.unmake_null_move();

            if (info.stopped) return 0;
            if (null_score >= beta) return beta;
        }
    }

    MoveList list;
    MoveGen::generate_legal_moves(board, list);

    if (list.count == 0) {
        if (in_check) {
            return -MATE_SCORE + ply; // Checkmate
        } else {
            return 0; // Stalemate
        }
    }

    score_moves(board, list, hash_move, ply, wd);

    int original_alpha = alpha;
    Move best_move = NULL_MOVE;
    int moves_searched = 0;

    for (int i = 0; i < list.count; ++i) {
        pick_move(list, i);
        Move m = list.moves[i];

        // Futility pruning near leaf nodes
        if (!is_pv && !in_check && depth <= 2 && moves_searched > 0 && !m.is_capture() && !m.is_promotion()) {
            if (static_eval + 150 * depth <= alpha) {
                continue;
            }
        }

        board.make_move(m);

        int score;
        if (moves_searched == 0) {
            // First move: full PVS window
            score = -negamax(board, depth - 1, -beta, -alpha, ply + 1, true, wd);
        } else {
            // Late Move Reductions (LMR)
            int reduction = 0;
            if (depth >= 3 && moves_searched >= 3 && !m.is_capture() && !in_check && !m.is_promotion()) {
                reduction = 1;
                if (depth >= 6 && moves_searched >= 6) reduction = 2;
            }

            score = -negamax(board, depth - 1 - reduction, -alpha - 1, -alpha, ply + 1, false, wd);

            if (score > alpha && reduction > 0) {
                // Re-search at full depth if reduced search beat alpha
                score = -negamax(board, depth - 1, -alpha - 1, -alpha, ply + 1, false, wd);
            }
            if (score > alpha && score < beta) {
                // Re-search with full window if PVS null-window failed high
                score = -negamax(board, depth - 1, -beta, -alpha, ply + 1, true, wd);
            }
        }

        board.unmake_move(m);
        moves_searched++;

        if (info.stopped) return 0;

        if (score > alpha) {
            alpha = score;
            best_move = m;

            if (score >= beta) {
                // Beta cutoff
                if (!m.is_capture() && ply < 128) {
                    wd.killer_moves[ply][1] = wd.killer_moves[ply][0];
                    wd.killer_moves[ply][0] = m;
                    wd.history_moves[board.turn()][m.from()][m.to()] += depth * depth;
                }

                g_tt.store(board.hash(), depth, TT_LOWERBOUND, beta, m, ply);
                return beta;
            }
        }
    }

    int flag = (alpha > original_alpha) ? TT_EXACT : TT_UPPERBOUND;
    g_tt.store(board.hash(), depth, flag, alpha, best_move, ply);

    return alpha;
}

void Searcher::helper_search(Board board, int start_depth, int thread_id) {
    WorkerData wd;
    wd.clear();

    for (int depth = start_depth; depth <= limits.depth && !info.stopped; ++depth) {
        negamax(board, depth, -INF, INF, 0, false, wd);
    }
}

static std::vector<Move> extract_pv(Board& board, int max_depth) {
    std::vector<Move> pv;
    std::vector<Move> made;
    for (int i = 0; i < max_depth; ++i) {
        Move m = g_tt.get_best_move(board.hash());
        if (m.is_null()) break;
        if (!board.make_move(m)) break;
        pv.push_back(m);
        made.push_back(m);
    }
    for (int i = static_cast<int>(made.size()) - 1; i >= 0; --i) {
        board.unmake_move(made[i]);
    }
    return pv;
}

Move Searcher::start_search(Board& board, const SearchLimits& search_limits) {
    stop(); // Ensure any previous threads joined
    reset();
    limits = search_limits;
    info.start_time = std::chrono::steady_clock::now();
    g_tt.new_search();

    MoveList root_moves;
    MoveGen::generate_legal_moves(board, root_moves);
    if (root_moves.count == 0) return NULL_MOVE;
    if (root_moves.count == 1) {
        std::cout << "bestmove " << move_to_uci(root_moves.moves[0]) << std::endl;
        return root_moves.moves[0];
    }

    Move best_overall_move = root_moves.moves[0];
    int max_d = limits.depth;

    // ELO adjustment: Limit depth and add controlled noise if limited
    if (limits.limit_strength && limits.elo < 4000) {
        if (limits.elo < 1200) max_d = std::min(max_d, 2);
        else if (limits.elo < 1600) max_d = std::min(max_d, 4);
        else if (limits.elo < 2000) max_d = std::min(max_d, 6);
        else if (limits.elo < 2500) max_d = std::min(max_d, 8);
        else if (limits.elo < 3200) max_d = std::min(max_d, 12);
        else if (limits.elo < 3800) max_d = std::min(max_d, 16);
    }

    // Launch Lazy SMP Helper Threads
    int num_threads = std::max(1, limits.num_threads);
    for (int i = 1; i < num_threads; ++i) {
        int thread_depth_offset = (i % 2 == 1) ? 2 : 1;
        helper_threads.emplace_back(&Searcher::helper_search, this, board, thread_depth_offset, i);
    }

    int alpha = -INF;
    int beta = INF;

    for (int depth = 1; depth <= max_d; ++depth) {
        info.depth = depth;
        info.seldepth = depth;

        // Aspiration Windows for depth >= 4
        int score = 0;
        if (depth >= 4) {
            int delta = 40;
            alpha = std::max(-INF, info.score - delta);
            beta = std::min(INF, info.score + delta);

            while (!info.stopped) {
                score = negamax(board, depth, alpha, beta, 0, true, main_worker);
                if (score <= alpha) {
                    alpha = std::max(-INF, alpha - delta);
                    delta *= 2;
                } else if (score >= beta) {
                    beta = std::min(INF, beta + delta);
                    delta *= 2;
                } else {
                    break;
                }
            }
        } else {
            score = negamax(board, depth, -INF, INF, 0, true, main_worker);
        }

        if (info.stopped) break;

        info.score = score;
        Move current_best = g_tt.get_best_move(board.hash());
        if (!current_best.is_null()) {
            best_overall_move = current_best;
        }

        // Print UCI info line
        auto now = std::chrono::steady_clock::now();
        long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.start_time).count();
        U64 total_n = info.nodes.load(std::memory_order_relaxed);
        long long nps = elapsed_ms > 0 ? (total_n * 1000) / elapsed_ms : 0;

        std::cout << "info depth " << depth
                  << " seldepth " << info.seldepth;
        
        if (std::abs(score) > MATE_SCORE - 100) {
            int mate_in = (MATE_SCORE - std::abs(score) + 1) / 2;
            if (score < 0) mate_in = -mate_in;
            std::cout << " score mate " << mate_in;
        } else {
            std::cout << " score cp " << score;
        }

        std::cout << " nodes " << total_n
                  << " nps " << nps
                  << " time " << elapsed_ms
                  << " pv";

        std::vector<Move> pv_line = extract_pv(board, depth);
        for (const auto& m : pv_line) {
            std::cout << " " << move_to_uci(m);
        }
        std::cout << std::endl;

        // If mate found, can break early
        if (std::abs(score) > MATE_SCORE - 100) break;
    }

    stop(); // Join helper threads

    // Apply ELO noise if lower ELO requested
    if (limits.limit_strength && limits.elo < 2500 && root_moves.count > 1) {
        static std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(0, 100);
        int blunder_chance = (2500 - limits.elo) / 35;
        if (dist(rng) < blunder_chance) {
            int pick = rng() % std::min(root_moves.count, 4);
            best_overall_move = root_moves.moves[pick];
        }
    }

    std::cout << "bestmove " << move_to_uci(best_overall_move) << std::endl;
    return best_overall_move;
}
