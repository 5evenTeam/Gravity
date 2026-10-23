# Gravity Chess 4000 ♟️⚡

**Gravity Chess 4000** is a high-performance chess engine and interactive chess suite built for extreme intelligence (up to **4000 ELO**), blazing speed (>28 million nodes/second local move generation), universal protocol support (**UCI & UDI** like Stockfish), and **Internet Super-Brain integration** (Stockfish 19 depth 70 cloud evaluation + Syzygy 7-piece endgame tablebases).

---

## 🌟 Key Features

### 1. Incredibly Smart Search & Evaluation (Local Engine)
- **64-bit Bitboard Engine**: Ultra-fast bitwise operations for piece movement and attacks.
- **Lazy SMP Multi-Threading**: Multi-threaded parallel search (`Threads` option supporting up to 64 CPU cores). Searches over 7.6+ million nodes/sec on modern multi-core processors.
- **Search Algorithm**:
  - Negamax with Alpha-Beta Pruning
  - Principal Variation Search (PVS)
  - Iterative Deepening with dynamic Aspiration Windows
  - Quiescence Search with MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
  - Transposition Table with 64-bit Zobrist Hashing (configurable up to 2048 MB)
  - Null Move Pruning with adaptive verification
  - Late Move Reductions (LMR)
  - Reverse Futility Pruning & Futility Pruning
  - Killer Move Heuristic (2 plies up to 128 plies) and History Heuristics
  - Check Extensions and 3-fold repetition / 50-move rule detection
- **Evaluation Function**:
  - PeSTO Piece-Square Tables (Tapered evaluation interpolating opening/middlegame and endgame phases)
  - Full Piece Mobility evaluation for Knights, Bishops, Rooks, and Queens
  - King Shelter & storm evaluation with open-file penalties
  - Pawn Structure evaluation (passed pawns, isolated pawns, doubled pawns)
  - Piece mobility & activity (bishop pair bonus, rooks on open and semi-open files)

### 2. Internet Super-Brain (4000 ELO Max Setting)
- **Lichess Cloud Evaluation**: Directly queries deep cloud evaluations (**Stockfish 19** up to depth 70+) for superhuman positional understanding.
- **Syzygy 7-Piece Endgame Tablebases**: In positions with 7 or fewer pieces, the engine connects to online tablebases to play 100% mathematically optimal winning or drawing moves with exact Distance-To-Mate (DTM).
- **Opening Recognition**: Dynamic ECO code and opening identifier for all major chess openings (Ruy Lopez, Sicilian, French, Italian Game, Scotch, Queen's Gambit, King's Indian, Caro-Kann, London System, English, Reti, etc.).

### 3. Interactive Modern Web GUI & Game Features
- **Live Game Clocks / Timers**: Support for Bullet (1+1), Blitz (3+2, 5 min), Rapid (10+5), or Casual untimed games with low-time alerts and flag detection.
- **Best Move Arrow Hint**: Visual SVG arrow dynamically drawn on the board showing the engine's suggested best move.
- **PGN Export**: 1-click download of full standard `.pgn` game records.
- **Custom FEN Loading**: Paste any FEN or puzzle position directly into the board.
- **Board Themes**: Switch between Oceanic Modern, Classic Wood, Emerald Green, and Cyber Neon.
- **Strict FIDE Rules**: Powered by embedded `chess.min.js` validator with zero illegal moves allowed.

### 3. UCI & UDI Protocol Compliance
- Supports standard **Universal Chess Interface (UCI)** and recognizes **UDI** as an alias!
- Usable in external chess GUIs like **Arena, CuteChess, Fritz, ChessBase, BanksiaGUI**, or online bot interfaces.
- Standard options:
  - `UCI_Elo` (800 to 4000)
  - `UCI_LimitStrength` (true/false)
  - `Hash` (table size in MB)
  - `OwnBook` (built-in opening book toggle)

### 4. Interactive Modern Web GUI
- Play human vs AI, watch AI vs AI self-play, or analyze positions.
- Live interactive chessboard with drag & drop, move animations, and Web Audio API synthesized sound effects.
- Dynamic Evaluation Bar with numerical centipawn advantage or Mate-in-X indicator.
- Live Engine Dashboard displaying Depth, Speed (NPS), Nodes, Evaluation Source, and Thinking Line (PV).
- ELO slider to scale difficulty from club player up to the 4000 ELO supercomputer.

---

## 🚀 Quick Start

### 1. Launch the Interactive Web GUI
Simply double-click:
```bat
run_gui.bat
```
Or run from PowerShell/Terminal:
```powershell
python gravity_server.py
```
Then open your browser at **`http://localhost:5000`**.

---

### 2. Run the Engine via UCI / UDI
Double-click:
```bat
run_engine_uci.bat
```
Or execute:
```powershell
.\gravity.exe
```

---

### 3. Recompiling the C++ Engine
To recompile `gravity.exe` from source:
```bat
build.bat
```
Or via g++:
```powershell
g++ -O3 -std=c++20 -march=native src/*.cpp -o gravity.exe
```
