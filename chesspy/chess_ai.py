import chess
import random 

class ChessAI:

    def __init__(self):
        self.board = chess.Chessboard()
    
    def decide_move(self):
        if len(self.board.available_moves) > 0:
            move = random.choice(self.board.available_moves)
            return move
        else:
            return None