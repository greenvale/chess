import wx
import chess
import random

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
                
                if self.board_comm["detail"]["select"] and (i,j) in self.board_comm["detail"]["select"]:
                    border_thickness = 6
                    border_color = wx.Colour(255,0,0)
                    
                elif self.board_comm["detail"]["select"] and (i,j) in self.board_comm["detail"]["highlight"]:
                    border_thickness = 4
                    border_color = wx.Colour(0,0,255)
                else:
                    border_thickness = 2
                    border_color = tile_color
                    
                dc.SetPen(wx.Pen(border_color, border_thickness))
                dc.SetBrush(wx.Brush(tile_color))
                dc.DrawRectangle(x + border_thickness//2, y + border_thickness//2, self.tile_size - border_thickness, self.tile_size - border_thickness)

                if self.board_comm["board"][i,j] and self.piece_images[self.board_comm["board"][i,j]]:
                    img = self.piece_images[self.board_comm["board"][i,j]]
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

        self.board = chess.create_board()
        self.board_comm = {"board": self.board, "detail":{"select":None, "highlight":None}}

        self.panel = ChessboardPanel(self, self.board_comm)

        chess.reset_board(self.board)

        self.selected_square = None
        self.playing = "w"
        self.moves = chess.get_available_moves(self.board, self.playing)

        self.Bind(wx.EVT_LEFT_DOWN, self.on_click)

        self.playing_mode = "auto"

        self.Show()
        
    def make_move(self, move):

        self.board[move["end"]] = self.board[move["start"]]
        self.board[move["start"]] = None
        self.playing = "w" if self.playing == "b" else "b"

        if self.playing_mode == "auto":
            self.moves = chess.get_available_moves(self.board, self.playing)

            self.panel.Refresh()

            move = random.choice(self.moves)
            self.board[move["end"]] = self.board[move["start"]]
            self.board[move["start"]] = None
            self.playing = "w" if self.playing == "b" else "b"

        self.moves = chess.get_available_moves(self.board, self.playing)


    def on_click(self, event):
        data = event.GetClientData()
        print(f"Clicked {data["idx"]}")
        idx = data["idx"]
        
        if self.selected_square:

            if self.selected_square == idx:
                # Have selected square and have reselected to same square so deselect

                self.selected_square = None
                self.board_comm["detail"]["highlight"].clear()
                self.board_comm["detail"]["select"].clear()

                #print(f"Deselected the selected square: selected square is {self.selected_square}")

            elif self.selected_square != idx and self.board[idx] and self.board[idx][1] == self.playing:
                # Have a selected square but have selected another player's piece so change the selected square to this

                self.selected_square = idx
                self.board_comm["detail"]["highlight"] = [x["end"] for x in self.moves if x["start"] == self.selected_square]
                self.board_comm["detail"]["select"] = [idx]

                #print(f"Changed to another square: selected square is {self.selected_square}")

            elif self.selected_square != idx and len([x for x in self.moves if x["start"]==self.selected_square and x["end"]==idx]) > 0:
                # Have selected square and have clicked a valid square for the selected piece to move to

                self.make_move({"start":self.selected_square, "end":idx})
                
                self.selected_square = None
                self.board_comm["detail"]["highlight"].clear()
                self.board_comm["detail"]["select"].clear()

                #print(f"Have clicked on valid end square, taking move. selected square {self.selected_square}")
                
        elif self.selected_square is None and self.board[idx][1] == self.playing:
            # Have no selected square but have selected a valid square containing piece owned by player so select this

            self.selected_square = idx
            self.board_comm["detail"]["highlight"] = [x["end"] for x in self.moves if x["start"] == self.selected_square]
            self.board_comm["detail"]["select"] = [idx]

            #print(f"No selected square, now selecting {self.selected_square}")

        self.panel.Refresh()


class ChessApp(wx.App):
    def OnInit(self):
        
        self.frame = ChessFrame()
        
        return True


if __name__ == "__main__":
    app = ChessApp()
    app.MainLoop()