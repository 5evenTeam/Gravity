import os
import sys
import json
import time
import subprocess
import urllib.request
import urllib.parse
from flask import Flask, request, jsonify, send_from_directory

app = Flask(__name__, static_folder='web')

ENGINE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'gravity.exe')
engine_process = None

def get_engine():
    global engine_process
    if engine_process is None or engine_process.poll() is not None:
        try:
            engine_process = subprocess.Popen(
                [ENGINE_PATH],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1
            )
            engine_process.stdin.write("uci\nisready\n")
            engine_process.stdin.flush()
        except Exception as e:
            print(f"Failed to start engine: {e}")
            engine_process = None
    return engine_process

def query_cloud_eval(fen):
    try:
        url = f"https://lichess.org/api/cloud-eval?fen={urllib.parse.quote(fen)}"
        req = urllib.request.Request(url, headers={'User-Agent': 'GravityChess/1.0'})
        with urllib.request.urlopen(req, timeout=1.8) as resp:
            data = json.loads(resp.read().decode('utf-8'))
            if 'pvs' in data and len(data['pvs']) > 0:
                pv = data['pvs'][0]
                moves = pv.get('moves', '').split()
                if moves:
                    depth = data.get('depth', 60)
                    return {
                        'best_move': moves[0],
                        'depth': depth,
                        'score_cp': pv.get('cp', 0),
                        'mate': pv.get('mate', None),
                        'pv': ' '.join(moves[:8]),
                        'source': f'Cloud Stockfish 19 (d={depth})'
                    }
    except Exception:
        pass
    return None

def query_tablebase(fen):
    piece_placement = fen.split()[0]
    pieces = [c for c in piece_placement if c.isalpha()]
    if len(pieces) > 7:
        return None

    try:
        url = f"https://tablebase.lichess.ovh/standard?fen={urllib.parse.quote(fen)}"
        req = urllib.request.Request(url, headers={'User-Agent': 'GravityChess/1.0'})
        with urllib.request.urlopen(req, timeout=1.8) as resp:
            data = json.loads(resp.read().decode('utf-8'))
            moves = data.get('moves', [])
            if moves:
                best = moves[0]
                dtm = best.get('dtm', 0)
                category = data.get('category', '')
                score = 0
                if category == 'win':
                    score = 15000 if dtm is None else (20000 - abs(dtm) * 10)
                elif category == 'loss':
                    score = -15000 if dtm is None else (-20000 + abs(dtm) * 10)
                
                return {
                    'best_move': best.get('uci'),
                    'depth': 100,
                    'score_cp': score,
                    'mate': dtm,
                    'pv': best.get('uci'),
                    'source': f'Syzygy Tablebase ({category.upper()})'
                }
    except Exception:
        pass
    return None

def query_local_engine(fen, moves=None, elo=4000, time_ms=800):
    eng = get_engine()
    if not eng:
        return {'best_move': None, 'error': 'Engine unavailable'}

    try:
        limit = "true" if elo < 4000 else "false"
        eng.stdin.write(f"setoption name UCI_LimitStrength value {limit}\n")
        eng.stdin.write(f"setoption name UCI_Elo value {elo}\n")

        if fen == 'startpos' or not fen:
            if moves:
                eng.stdin.write(f"position startpos moves {' '.join(moves)}\n")
            else:
                eng.stdin.write("position startpos\n")
        else:
            if moves:
                eng.stdin.write(f"position fen {fen} moves {' '.join(moves)}\n")
            else:
                eng.stdin.write(f"position fen {fen}\n")

        eng.stdin.write(f"go movetime {time_ms}\n")
        eng.stdin.flush()

        best_move = None
        depth = 0
        score_cp = 0
        mate = None
        nodes = 0
        nps = 0
        pv = ""

        start_wait = time.time()
        while time.time() - start_wait < 5.0:
            line = eng.stdout.readline()
            if not line:
                break
            line = line.strip()

            if line.startswith("info"):
                tokens = line.split()
                if "depth" in tokens:
                    depth = int(tokens[tokens.index("depth") + 1])
                if "score" in tokens:
                    idx = tokens.index("score")
                    stype = tokens[idx + 1]
                    sval = int(tokens[idx + 2])
                    if stype == "cp":
                        score_cp = sval
                        mate = None
                    elif stype == "mate":
                        mate = sval
                if "nodes" in tokens:
                    nodes = int(tokens[tokens.index("nodes") + 1])
                if "nps" in tokens:
                    nps = int(tokens[tokens.index("nps") + 1])
                if "pv" in tokens:
                    pv_idx = tokens.index("pv")
                    pv = " ".join(tokens[pv_idx + 1:pv_idx + 8])

            if line.startswith("bestmove"):
                best_move = line.split()[1]
                break

        return {
            'best_move': best_move,
            'depth': depth,
            'score_cp': score_cp,
            'mate': mate,
            'nodes': nodes,
            'nps': nps,
            'pv': pv,
            'source': f'Gravity (ELO {elo})'
        }
    except Exception as e:
        return {'best_move': None, 'error': str(e)}

@app.route('/')
def index():
    return send_from_directory('web', 'index.html')

@app.route('/<path:path>')
def static_files(path):
    return send_from_directory('web', path)

@app.route('/chess.min.js')
def chess_js():
    return send_from_directory('web', 'chess.min.js')

@app.route('/api/move', methods=['POST'])
def make_move_api():
    data = request.json or {}
    fen = data.get('fen', 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1')
    moves = data.get('moves', [])
    elo = int(data.get('elo', 4000))
    use_cloud = data.get('use_cloud', True)
    movetime = int(data.get('movetime', 700))

    result = None

    if elo >= 3500 and use_cloud:
        result = query_tablebase(fen)
        if not result:
            result = query_cloud_eval(fen)

    if not result or not result.get('best_move'):
        result = query_local_engine(fen, moves=moves, elo=elo, time_ms=movetime)

    return jsonify(result)

@app.route('/api/status', methods=['GET'])
def engine_status():
    eng = get_engine()
    alive = eng is not None and eng.poll() is None
    return jsonify({
        'status': 'online',
        'engine_connected': alive,
        'engine_name': 'Gravity',
        'max_elo': 4000
    })

if __name__ == '__main__':
    port = int(os.environ.get('PORT', 5000))
    print(f"Gravity server listening on http://localhost:{port}")
    app.run(host='0.0.0.0', port=port, debug=False)
