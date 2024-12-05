import wx
import chess
import random
import chess_ai

class ChessboardPanel(wx.Panel):

    def __init__(self, parent, board_comm):

        super().__init__(parent)

        self.load_piece_images()

        self.board_comm = board_comm

        self.Bind(wx.EVT_PAINT, self.on_paint)
        self.Bind(wx.EVT_SIZE, self.on_resize)
        self.Bind(wx.EVT_LEFT_DOWN, self.on_click)
        
    def on_resize(self, event):
        self.Refresh()
        event.Skip()

    def on_paint(self, event):
        dc = wx.PaintDC(self)
        self.draw_chessboard(dc)

    def load_piece_images(self):
        self.piece_images = {}
        labels = [ p+o for p in ["p", "n", "b", "r", "q", "k"] for o in ["w", "b"] ]
        for l in labels:
            path = "./img/" + l + ".png"
            try:
                self.piece_images[l] = wx.Bitmap(path)
            except Exception as e:
                print(f"error loading {l} image")
                self.piece_images[l] = None


    def draw_chessboard(self, dc):
        width, height = dc.GetSize()

        self.tile_size = min(width, height) // 8
        self.x_offset = (width - self.tile_size*8) // 2
        self.y_offset = (height - self.tile_size*8) // 2

        for row in range(8):
            for col in range(8):
                
                i = col
                j = 7 - row

                x = self.x_offset + col*self.tile_size
                y = self.y_offset + row*self.tile_size

                is_white = (i + j) % 2 == 0

                default_white = wx.Colour(230, 210, 180)
                default_black = wx.Colour(150, 130, 100)

                tile_color = default_white if is_white else default_black
                
                if self.board_comm["select"] and (i,j) in self.board_comm["select"]:
                    border_thickness = 6
                    border_color = wx.Colour(255,0,0)
                    
                elif self.board_comm["select"] and (i,j) in self.board_comm["highlight"]:
                    border_thickness = 4
                    border_color = wx.Colour(0,0,255)
                else:
                    border_thickness = 2
                    border_color = tile_color
                    
                dc.SetPen(wx.Pen(border_color, border_thickness))
                dc.SetBrush(wx.Brush(tile_color))
                dc.DrawRectangle(x + border_thickness//2, y + border_thickness//2, self.tile_size - border_thickness, self.tile_size - border_thickness)

                square_data = self.Parent.board.board[i,j]

                if square_data and self.piece_images[square_data]:
                    img = self.piece_images[square_data]
                    img = img.ConvertToImage()
                    img = img.Rescale(self.tile_size - 8, self.tile_size - 8)
                    img = wx.Bitmap(img)
                    dc.DrawBitmap(img, x + 4, y + 4, True)


    def on_click(self, event):
        mx, my = event.GetPosition()
        i = (mx - self.x_offset) // self.tile_size
        j = 7 - ((my - self.y_offset) // self.tile_size)

        if 0<=i<8 and 0<=j<8:
            new_event = wx.PyCommandEvent(wx.EVT_LEFT_DOWN.typeId, self.GetId())
            new_event.SetEventObject(self)
            new_event.SetClientData({"idx": (i,j)})
            wx.PostEvent(self.GetParent(), new_event)


class ChessFrame(wx.Frame):
    def __init__(self):
        super().__init__(None, title="Chessboard", size=(900, 700))

        self.board = chess.Chessboard()
        self.AI = chess_ai.ChessAI()
        self.board_comm = {"select":None, "highlight":None}
        self.panel = ChessboardPanel(self, self.board_comm)

        self.selected_square = None

        self.playing_mode = "auto"

        self.Bind(wx.EVT_LEFT_DOWN, self.on_click)

        self.Show()


    def make_move(self, move):

        self.board.move(move["start"], move["end"])

        if self.playing_mode == "auto":

            self.panel.Refresh()

            self.AI.copy_board(self.board)
            #move = self.AI.decide_move()
            move = random.choice(self.board.available_moves)

            self.board.move(move["start"], move["end"])
        

    def on_click(self, event):
        data = event.GetClientData()
        #print(f"Clicked {data["idx"]}")
        idx = data["idx"]
        
        if self.selected_square:

            if self.selected_square == idx:
                # Have selected square and have reselected to same square so deselect

                self.selected_square = None
                self.board_comm["highlight"].clear()
                self.board_comm["select"].clear()

            elif self.selected_square != idx and self.board.board[idx] and self.board.board[idx][1] == self.board.playing:
                # Have a selected square but have selected another player's piece so change the selected square to this

                self.selected_square = idx
                self.board_comm["highlight"] = [x["end"] for x in self.board.available_moves if x["start"] == self.selected_square]
                self.board_comm["select"] = [idx]

            elif self.selected_square != idx and len([x for x in self.board.available_moves if x["start"]==self.selected_square and x["end"]==idx]) > 0:
                # Have selected square and have clicked a valid square for the selected piece to move to

                move = {"start":self.selected_square, "end":idx}

                self.selected_square = None
                self.board_comm["highlight"].clear()
                self.board_comm["select"].clear()

                self.make_move(move)


        elif self.selected_square is None and self.board.board[idx][1] == self.board.playing:
            # Have no selected square but have selected a valid square containing piece owned by player so select this

            self.selected_square = idx
            self.board_comm["highlight"] = [x["end"] for x in self.board.available_moves if x["start"] == self.selected_square]
            self.board_comm["select"] = [idx]

        self.panel.Refresh()


class ChessApp(wx.App):
    def OnInit(self):
        
        self.frame = ChessFrame()
        
        return True


if __name__ == "__main__":
    app = ChessApp()
    app.MainLoop()