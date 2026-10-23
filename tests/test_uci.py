"""Black-box UCI regression tests for a built Gravity executable.

Run after compiling with: python -m unittest discover -s tests -v
"""

from pathlib import Path
import os
import subprocess
import unittest


ROOT = Path(__file__).resolve().parents[1]
ENGINE = Path(os.environ.get("GRAVITY_ENGINE", ROOT / "gravity.exe"))


def run_engine(commands: str) -> str:
    completed = subprocess.run(
        [str(ENGINE)],
        input=commands,
        text=True,
        capture_output=True,
        cwd=ROOT,
        timeout=20,
        check=False,
    )
    if completed.returncode != 0:
        raise AssertionError(completed.stderr or f"engine exited {completed.returncode}")
    return completed.stdout


@unittest.skipUnless(ENGINE.is_file(), "build gravity.exe before running UCI tests")
class UCITests(unittest.TestCase):
    def test_handshake_and_depth_search(self):
        output = run_engine(
            "uci\n"
            "isready\n"
            "setoption name OwnBook value false\n"
            "setoption name Threads value 1\n"
            "position startpos\n"
            "go depth 2\n"
            "quit\n"
        )
        self.assertIn("uciok", output)
        self.assertIn("readyok", output)
        self.assertRegex(output, r"bestmove [a-h][1-8][a-h][1-8][qrbn]?")

    def test_start_position_perft_three(self):
        output = run_engine("position startpos\nperft 3\nquit\n")
        self.assertIn("Total nodes: 8902", output)

    def test_node_limited_search_returns_a_move(self):
        output = run_engine(
            "setoption name OwnBook value false\n"
            "setoption name Threads value 1\n"
            "position startpos\n"
            "go nodes 100\n"
            "quit\n"
        )
        self.assertRegex(output, r"bestmove [a-h][1-8][a-h][1-8][qrbn]?")


if __name__ == "__main__":
    unittest.main()
