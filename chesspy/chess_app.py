import wx
import chess
import random
import chess_ai
import wx.lib.newevent


UndoEvent, EVT_UNDO = wx.lib.newevent.NewEvent()
ChangeModeEvent, EVT_CHANGE_MODE = wx.lib.newevent.NewEvent()
BoardClickEvent, EVT_BOARD_CLICK = wx.lib.newevent.NewEvent()


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
                
                if self.board_comm["red_square"] and (i,j) in self.board_comm["red_square"]:
                    tile_color = wx.Colour(230,0,0) if is_white else wx.Colour(150,0,0)
                if self.board_comm["select"] and (i,j) in self.board_comm["select"]:
                    border_thickness = 6
                    border_color = wx.Colour(255,0,0)
                elif self.board_comm["highlight"] and (i,j) in self.board_comm["highlight"]:
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
            board_click_event = BoardClickEvent(message=(i,j))
            wx.PostEvent(self.GetParent(), board_click_event)
        
        event.Skip()


class ControlPanel(wx.Panel):
    def __init__(self, parent):
        super().__init__(parent)
        self.undo_button = wx.Button(self, label="Undo")
        self.undo_button.Bind(wx.EVT_BUTTON, self.on_undo_click)

        opposite_mode = "manual" if self.GetParent().playing_mode == "auto" else "auto"
        self.change_mode_button = wx.Button(self, label="Change to "+opposite_mode)
        self.change_mode_button.Bind(wx.EVT_BUTTON, self.on_change_mode_click)

        self.text_box = wx.StaticText(self, label="In Progress")
        self.text_box.SetWindowStyle(wx.BORDER_SIMPLE)
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.undo_button, 0, wx.ALL | wx.ALIGN_CENTER_HORIZONTAL, 10)  # Add button with margin
        sizer.Add(self.change_mode_button, 0, wx.ALL | wx.ALIGN_CENTER_HORIZONTAL, 10)
        sizer.Add(self.text_box, 0, wx.ALL | wx.ALIGN_CENTER_HORIZONTAL, 10)  # Add text box with margin
        self.SetSizer(sizer)

    def on_undo_click(self, event):
        undo_event = UndoEvent(message="Undo button clicked")
        wx.PostEvent(self.GetParent(), undo_event)     
        event.Skip()

    def on_change_mode_click(self, event):
        change_mode_event = ChangeModeEvent(message="Change mode")
        self.change_mode_button.SetLabel("Change to " + self.GetParent().playing_mode)
        wx.PostEvent(self.GetParent(), change_mode_event)
        event.Skip()

class ChessFrame(wx.Frame):
    def __init__(self):
        super().__init__(None, title="Chessboard", size=(600, 400))

        self.playing_mode = "auto"
        self.board = chess.Chessboard()
        self.AI = chess_ai.ChessAI()
        self.board_comm = {"select":[], "highlight":[], "red_square":[]}
        self.board_panel = ChessboardPanel(self, self.board_comm)

        self.control_panel = ControlPanel(self)
        self.control_panel.SetWindowStyle(wx.BORDER_SIMPLE)
        self.board_panel.SetWindowStyle(wx.BORDER_SIMPLE)

        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(self.board_panel, 7, wx.EXPAND | wx.ALL, 5)
        sizer.Add(self.control_panel, 3, wx.EXPAND | wx.ALL, 5)

        self.selected_square = None

        self.Bind(EVT_BOARD_CLICK, self.on_click)
        self.Bind(EVT_UNDO, self.on_undo_event)
        self.Bind(EVT_CHANGE_MODE, self.on_change_mode_event)
        self.SetSizer(sizer)
        self.Show()


    def make_move(self, start, end):

        self.board.move(start, end)
        self.board.print_status()  

        if self.playing_mode == "auto":

            self.board_panel.Refresh()

            self.AI.board.copy_board(self.board)
            move = self.AI.decide_move()
            if move:
                self.board.move(move["start"], move["end"])
                self.board.print_status() 
            else:
                print("Cannot make any moves")


    def on_change_mode_event(self, event):
        self.playing_mode = "manual" if self.playing_mode == "auto" else "auto"
        print(f"Mode is {self.playing_mode}")

    def on_undo_event(self, event):
        
        self.selected_square = None
        self.board_comm["highlight"].clear()
        self.board_comm["select"].clear()

        if self.playing_mode == "auto":
            self.board.undo_move()
            self.board.print_status()
        self.board.undo_move()
        self.board.print_status()

        self.board_panel.Refresh()
        self.control_panel.Refresh()


    def on_click(self, event):
        idx = event.message
        print(f"Clicked square {idx}")
        if self.selected_square:

            if self.selected_square == idx:
                # Have selected square and have reselected to same square so deselect

                self.selected_square = None
                self.board_comm["highlight"].clear()
                self.board_comm["select"].clear()

            elif self.selected_square != idx and self.board.board[idx] and self.board.board[idx][1] == self.board.playing:
                # Have a selected square but have selected another of playing player's piece so change the selected square to this

                self.selected_square = idx
                self.board_comm["highlight"] = [x["end"] for x in self.board.available_moves if x["start"] == self.selected_square]
                self.board_comm["select"] = [idx]

            elif self.selected_square != idx and len([x for x in self.board.available_moves if x["start"]==self.selected_square and x["end"]==idx]) > 0:
                # Have selected square and have clicked a valid square for the selected piece to move to

                self.make_move(self.selected_square, idx)

                self.selected_square = None
                self.board_comm["highlight"].clear()
                self.board_comm["select"].clear()
                


        elif self.selected_square is None and self.board.board[idx] and self.board.board[idx][1] == self.board.playing:
            # Have no selected square but have selected a valid square containing piece owned by player so select this

            self.selected_square = idx
            self.board_comm["highlight"] = [x["end"] for x in self.board.available_moves if x["start"] == self.selected_square]
            self.board_comm["select"] = [idx]
        
        self.board_panel.Refresh()
        self.control_panel.Refresh()


class ChessApp(wx.App):
    def OnInit(self):
        
        self.frame = ChessFrame()
        
        return True


if __name__ == "__main__":
    app = ChessApp()
    app.MainLoop()