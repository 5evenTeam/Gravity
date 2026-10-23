# Gravity

A chess engine written in C++ with 64-bit bitboards, alpha-beta search, and UCI protocol support. Includes a local web interface with game clocks, analysis tools, and optional cloud evaluation integration.

![Gravity Logo](logo.png)

## Overview

Gravity is a UCI-compliant chess engine featuring bitboard move generation, tapered piece-square evaluation, and parallel search using Lazy SMP.

### Engine Details
- **Representation**: 64-bit bitboards with ray-cast sliding attacks
- **Search**:
  - Negamax with alpha-beta pruning
  - Principal Variation Search (PVS)
  - Iterative deepening with aspiration windows
  - Quiescence search with MVV-LVA move ordering
  - Transposition table with 64-bit Zobrist hashing
  - Null-move pruning and late move reductions (LMR)
  - Reverse futility pruning and killer move heuristics (2 plies)
  - Multi-threading via Lazy SMP
- **Evaluation**:
  - PeSTO piece-square tables with game-phase interpolation
  - Piece mobility for knights, bishops, rooks, and queens
  - Pawn structure evaluation (passed, isolated, and doubled pawns)
  - King shelter and file safety penalties
- **Protocols**: Full UCI compliance (also accepts `udi` alias)

### Web Interface
- Board with legal move indicators and drag-and-drop
- Move history in standard algebraic notation
- Configurable game clocks (Bullet, Blitz, Rapid, Casual)
- Live evaluation bar and best-move indicator
- PGN export and FEN import/export
- Multiple board color schemes

## Requirements

- **Engine**: C++20 compiler (`g++`, `clang++`, or MSVC)
- **Web UI**: Python 3.8+ with `flask`

## Build

Compile the engine with MinGW or g++:

```bash
g++ -O3 -std=c++20 -march=native src/*.cpp -o gravity.exe
```

Or run `build.bat` on Windows.

## Usage

### Web Interface
Start the local server and open `http://localhost:5000`:

```bash
python gravity_server.py
```

Or run `run_gui.bat`.

### Command Line / UCI
Run the engine directly or connect it to any chess GUI (Arena, CuteChess, Banksia, Fritz):

```bash
./gravity.exe
```

Example UCI session:
```
uci
isready
position startpos moves e2e4 e7e5
go depth 10
quit
```

Benchmark move generation:
```
perft 5
```

## License

MIT
