#include "uci.h"
#include "book.h"
#include "movegen.h"
#include "evaluate.h"
#include "tt.h"
#include <sstream>
#include <iostream>
#include <thread>

UCI::UCI() : own_book(true) {
    limits.elo = 4000;
    limits.limit_strength = false;
}

void UCI::parse_position(std::istringstream& ss) {
    std::string token;
    ss >> token;

    if (token == "startpos") {
        board.set_fen(START_FEN);
        ss >> token; // Check for "moves"
    } else if (token == "fen") {
        std::string fen = "";
        while (ss >> token && token != "moves") {
            if (!fen.empty()) fen += " ";
            fen += token;
        }
        board.set_fen(fen);
    }

    if (token == "moves") {
        std::string move_str;
        while (ss >> move_str) {
            MoveList list;
            MoveGen::generate_legal_moves(board, list);
            bool found = false;
            for (int i = 0; i < list.count; ++i) {
                if (move_to_uci(list.moves[i]) == move_str) {
                    board.make_move(list.moves[i]);
                    found = true;
                    break;
                }
            }
            if (!found) break;
        }
    }
}

void UCI::parse_go(std::istringstream& ss) {
    limits.time_ms = -1;
    limits.depth = 64;
    limits.max_nodes = -1;
    limits.infinite = false;

    int wtime = -1, btime = -1, winc = 0, binc = 0, movetime = -1;
    int depth = -1, nodes = -1, movestogo = 0;
    std::string token;

    while (ss >> token) {
        if (token == "wtime") ss >> wtime;
        else if (token == "btime") ss >> btime;
        else if (token == "winc") ss >> winc;
        else if (token == "binc") ss >> binc;
        else if (token == "movetime") ss >> movetime;
        else if (token == "depth") ss >> depth;
        else if (token == "nodes") ss >> nodes;
        else if (token == "movestogo") ss >> movestogo;
        else if (token == "infinite") limits.infinite = true;
    }

    if (movetime > 0) {
        limits.time_ms = movetime;
    } else if (!limits.infinite) {
        const int remaining = board.turn() == WHITE ? wtime : btime;
        const int increment = board.turn() == WHITE ? winc : binc;
        if (remaining > 0) {
            const int moves_left = movestogo > 0 ? movestogo : 25;
            const int allocation = (remaining / moves_left) + (increment * 3 / 4);
            limits.time_ms = std::max(20, std::min(allocation, std::max(20, remaining - 25)));
        }
    }

    if (depth > 0) {
        limits.depth = depth;
    }
    if (nodes > 0) {
        limits.max_nodes = nodes;
    }

    // Check opening book first
    if (own_book) {
        std::string op_name, op_eco;
        Move bm = Book::get_book_move(board.get_fen(), &op_name, &op_eco);
        if (!bm.is_null()) {
            std::cout << "info string Book move (" << op_eco << " " << op_name << ")" << std::endl;
            std::cout << "bestmove " << move_to_uci(bm) << std::endl;
            return;
        }
    }

    searcher.start_search(board, limits);
}

void UCI::parse_setoption(std::istringstream& ss) {
    std::string token, name, value;
    ss >> token; // "name"
    while (ss >> token && token != "value") {
        if (!name.empty()) name += " ";
        name += token;
    }
    ss >> value;

    if (name == "Clear Hash") {
        g_tt.clear();
    } else if (name == "Hash" && !value.empty()) {
        int mb = std::stoi(value);
        g_tt.resize(std::max(1, std::min(mb, 2048)));
    } else if (name == "UCI_Elo" && !value.empty()) {
        limits.elo = std::max(800, std::min(std::stoi(value), 4000));
    } else if (name == "UCI_LimitStrength" && !value.empty()) {
        limits.limit_strength = (value == "true" || value == "1");
    } else if (name == "OwnBook" && !value.empty()) {
        own_book = (value == "true" || value == "1");
    } else if (name == "Threads" && !value.empty()) {
        limits.num_threads = std::max(1, std::min(std::stoi(value), 64));
    }
}

U64 UCI::perft_recursive(Board& b, int depth) {
    if (depth == 0) return 1ULL;
    MoveList list;
    MoveGen::generate_legal_moves(b, list);
    if (depth == 1) return list.count;

    U64 nodes = 0;
    for (int i = 0; i < list.count; ++i) {
        b.make_move(list.moves[i]);
        nodes += perft_recursive(b, depth - 1);
        b.unmake_move(list.moves[i]);
    }
    return nodes;
}

void UCI::run_perft(int depth) {
    std::cout << "Running Perft depth " << depth << "..." << std::endl;
    auto t0 = std::chrono::steady_clock::now();
    MoveList list;
    MoveGen::generate_legal_moves(board, list);
    U64 total_nodes = 0;

    for (int i = 0; i < list.count; ++i) {
        board.make_move(list.moves[i]);
        U64 branch_nodes = perft_recursive(board, depth - 1);
        board.unmake_move(list.moves[i]);
        total_nodes += branch_nodes;
        std::cout << move_to_uci(list.moves[i]) << ": " << branch_nodes << "\n";
    }

    auto t1 = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "\nTotal nodes: " << total_nodes
              << " in " << ms << " ms ("
              << (ms > 0 ? (total_nodes * 1000) / ms : 0) << " nps)\n" << std::endl;
}

void UCI::run_benchmark() {
    board.set_fen(START_FEN);
    run_perft(5);
}

void UCI::loop() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "uci" || cmd == "udi") {
            std::cout << "id name Gravity Chess 4000" << std::endl;
            std::cout << "id author Gravity Team" << std::endl;
            std::cout << "option name Hash type spin default 64 min 1 max 2048" << std::endl;
            std::cout << "option name UCI_Elo type spin default 4000 min 800 max 4000" << std::endl;
            std::cout << "option name UCI_LimitStrength type check default false" << std::endl;
            std::cout << "option name OwnBook type check default true" << std::endl;
            std::cout << "option name Threads type spin default 4 min 1 max 64" << std::endl;
            std::cout << "option name Clear Hash type button" << std::endl;
            std::cout << (cmd == "udi" ? "udiok" : "uciok") << std::endl;
        } else if (cmd == "isready") {
            std::cout << "readyok" << std::endl;
        } else if (cmd == "ucinewgame" || cmd == "udinewgame") {
            g_tt.clear();
            board.set_fen(START_FEN);
        } else if (cmd == "position") {
            parse_position(ss);
        } else if (cmd == "go") {
            parse_go(ss);
        } else if (cmd == "stop") {
            searcher.stop();
        } else if (cmd == "setoption") {
            parse_setoption(ss);
        } else if (cmd == "d" || cmd == "print") {
            board.print();
        } else if (cmd == "eval") {
            std::cout << "Static evaluation: " << Eval::evaluate(board) << " cp" << std::endl;
        } else if (cmd == "perft") {
            int d = 4;
            ss >> d;
            run_perft(d);
        } else if (cmd == "quit") {
            break;
        }
    }
}
