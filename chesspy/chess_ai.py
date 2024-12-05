import chess

class ChessAI:

    def __init__(self):
        self.board = chess.Chessboard()
        
    def copy_board(self, board : chess.Chessboard):        
        for i in range(8):
            for j in range(8):
                self.board.board[i,j] = board.board[i,j]
        self.board.history.clear()
    
    def decide_move(self):

        pass