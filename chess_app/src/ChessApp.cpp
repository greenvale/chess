#include "../../chessboard/chessboard.h"
#include <wx/wx.h>
#include <thread>
#include <functional>


struct Comm
{
    std::function<void(chessboard::Move)> requestMove;
};

/**************************************************************************************************************/
// BOARD PANEL

class BoardPanel : public wxPanel
{
private:
    int pos_x, pos_y;
    int size;

    Comm* comm;

    chessboard::Board* board;
    chessboard::Player playerView;

    wxColour light = wxColour(100, 100, 100);
    wxColour dark = wxColour(200, 200, 200);

    std::unordered_map<chessboard::Piece, wxImage> whitePieceImgs;
    std::unordered_map<chessboard::Piece, wxImage> blackPieceImgs;

    bool mouseIsDown = false;
    bool activeSqr = false;
    int mouseDown_x, mouseDown_y;
    int mouseUp_x, mouseUp_y;
    chessboard::GridVector gvDown, gvUp, startSqr, endSqr;
    
    chessboard::GridVector pos2gv(int x, int y);
    chessboard::GridVector ind2gv(int x, int y);
    int gv2i(chessboard::GridVector gv);
    int gv2j(chessboard::GridVector gv);

public:
    BoardPanel(wxWindow* parent, chessboard::Board* board, chessboard::Player playerView, Comm* comm);
    ~BoardPanel();

    void step();
    void setPanelDims(int x, int y, int width);

    void onMouseDown(wxMouseEvent& evt); 
    void onMouseUp(wxMouseEvent& evt);
    void onMouseMove(wxMouseEvent& evt);
    void onPaint(wxPaintEvent& evt);

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(BoardPanel, wxPanel)
    EVT_LEFT_DOWN(BoardPanel::onMouseDown)
    EVT_LEFT_UP(BoardPanel::onMouseUp)
    EVT_MOTION(BoardPanel::onMouseMove)
    EVT_PAINT(BoardPanel::onPaint)
END_EVENT_TABLE()

BoardPanel::BoardPanel(wxWindow* parent, chessboard::Board* board, chessboard::Player playerView, Comm* comm) : wxPanel(parent, wxID_ANY, wxPoint(0, 0), wxSize(0, 0))
{
    this->board = board;
    this->playerView = playerView;
    this->comm = comm;

    // load piece images
    this->whitePieceImgs[chessboard::PAWN].LoadFile(wxString("img/Chess_plt60.png"), wxBITMAP_TYPE_PNG);
    this->blackPieceImgs[chessboard::PAWN].LoadFile(wxString("img/Chess_pdt60.png"), wxBITMAP_TYPE_PNG);
    this->whitePieceImgs[chessboard::ROOK].LoadFile(wxString("img/Chess_rlt60.png"), wxBITMAP_TYPE_PNG);
    this->blackPieceImgs[chessboard::ROOK].LoadFile(wxString("img/Chess_rdt60.png"), wxBITMAP_TYPE_PNG);
    this->whitePieceImgs[chessboard::KNIGHT].LoadFile(wxString("img/Chess_nlt60.png"), wxBITMAP_TYPE_PNG);
    this->blackPieceImgs[chessboard::KNIGHT].LoadFile(wxString("img/Chess_ndt60.png"), wxBITMAP_TYPE_PNG);
    this->whitePieceImgs[chessboard::BISHOP].LoadFile(wxString("img/Chess_blt60.png"), wxBITMAP_TYPE_PNG);
    this->blackPieceImgs[chessboard::BISHOP].LoadFile(wxString("img/Chess_bdt60.png"), wxBITMAP_TYPE_PNG);
    this->whitePieceImgs[chessboard::QUEEN].LoadFile(wxString("img/Chess_qlt60.png"), wxBITMAP_TYPE_PNG);
    this->blackPieceImgs[chessboard::QUEEN].LoadFile(wxString("img/Chess_qdt60.png"), wxBITMAP_TYPE_PNG);
    this->whitePieceImgs[chessboard::KING].LoadFile(wxString("img/Chess_klt60.png"), wxBITMAP_TYPE_PNG);
    this->blackPieceImgs[chessboard::KING].LoadFile(wxString("img/Chess_kdt60.png"), wxBITMAP_TYPE_PNG);

    // report status of images loaded
    for (auto& img : this->whitePieceImgs)
    {
        if (img.second.IsOk())
        {
            std::cout << "White " << img.first << " loaded" << std::endl;
        }
        else
        {
            std::cout << "White " << img.first << " not loaded" << std::endl;
        }   
    }
    for (auto& img : this->whitePieceImgs)
    {
        if (img.second.IsOk())
        {
            std::cout << "Black " << img.first << " loaded" << std::endl;
        }
        else
        {
            std::cout << "Black " << img.first << " not loaded" << std::endl;
        }   
    }
}

BoardPanel::~BoardPanel()
{

}

void BoardPanel::step()
{
    this->activeSqr = false;
    this->Refresh();
    this->Update();
}

void BoardPanel::setPanelDims(int x, int y, int size)
{
    this->pos_x = x;
    this->pos_y = y;
    this->size = size;
    this->SetSize(pos_x, pos_y, size, size);
}

void BoardPanel::onMouseDown(wxMouseEvent& evt)
{
    this->CaptureMouse();
    this->mouseDown_x = evt.GetX();
    this->mouseDown_y = evt.GetY();
    this->gvDown = this->pos2gv(this->mouseDown_x, this->mouseDown_y);
    this->mouseIsDown = true;
}

void BoardPanel::onMouseUp(wxMouseEvent& evt)
{
    this->ReleaseMouse();
    this->mouseUp_x = evt.GetX();
    this->mouseUp_y = evt.GetY();
    this->gvUp = this->pos2gv(this->mouseUp_x, this->mouseUp_y);
    this->mouseIsDown = false;

    if (this->gvUp != chessboard::GridVector(999,999) && this->board->getPlayerToMove() == this->playerView)
    {
        if (this->gvUp == this->gvDown)
        {
            // CLICKED on square
            if (this->activeSqr == false)
            {
                // no current active square, activate this square
                if (this->board->getSqrOwner(this->gvUp) == this->playerView)
                {
                    this->startSqr = this->gvUp;
                    this->activeSqr = true;
                    std::cout << "Activated " << this->gvUp << std::endl;
                }
            }
            else
            {
                // if the clicked square is the same as the active square then cancel the activation
                if (this->gvUp == this->startSqr)
                {
                    // cancelling move
                    std::cout << "Cancelled move" << std::endl;
                    this->activeSqr = false;
                }
                else
                {
                    
                    if (this->board->getSqrOwner(this->gvUp) == this->playerView)
                    {
                        // if alternative square has been activated instead
                        this->startSqr = this->gvUp;
                        std::cout << "Alternative activated: " << this->gvUp << std::endl;
                    }
                    else //if (this->board->getSqrOwner(this->startSqr) == this->playerView)
                    {
                        // if square that isn't owned by player in view is clicked, check if it is option for this square
                        this->endSqr = this->gvUp;
                        std::cout << "(Select) Checking move: " << this->startSqr << ", " << this->endSqr << std::endl;

                        // check if move is valid move for this square
                        std::vector<chessboard::Move> validMoves = this->board->getValidMoves(this->startSqr);
                        bool validMove = false;
                        for (auto& mv : validMoves)
                        {
                            if (mv.end == this->endSqr)
                            {
                                validMove = true;
                            }
                        }

                        if (validMove == true)
                        {
                            // if valid move, then make request on comm for the main panel to communicate with board
                            std::cout << "Move is valid, placing request through comm" << std::endl;
                            this->comm->requestMove(chessboard::Move(this->startSqr, this->endSqr));
                            this->activeSqr = false;
                        }
                        else
                        {
                            // if not valid move then do nothing, keep active square active
                            std::cout << "Move not valid" << std::endl;
                        }                    
                    }
                    //else
                    //{
                       // std::cout << "Square not owned by player with view" << std::endl;
                    //}
                }
            }
        }
        else
        {
            // DRAGGED piece from one square to another
            this->startSqr = this->gvDown;
            this->endSqr = this->gvUp;
            std::cout << "(Drag) Checking move: " << this->startSqr << ", " << this->endSqr << std::endl;

            if (this->board->getSqrOwner(this->startSqr) == this->playerView)
            {
                // check if move is valid move for this square
                std::vector<chessboard::Move> validMoves = this->board->getValidMoves(this->startSqr);
                bool validMove = false;
                for (auto& mv : validMoves)
                {
                    if (mv.end == this->endSqr)
                    {
                        validMove = true;
                    }
                }

                if (validMove == true)
                {
                    // if valid move, then make request on comm for the main panel to communicate with board
                    std::cout << "Move is valid, placing request through comm" << std::endl;
                    this->comm->requestMove(chessboard::Move(this->startSqr, this->endSqr));
                }
                else
                {
                    // if not valid move then do nothing, keep active square active
                    std::cout << "Move not valid" << std::endl;
                } 
            }
            else
            {
                std::cout << "Square not owned by player holding view" << std::endl;
            }
        }
    }
    else
    {
        // mouse trajectory not valid
        std::cout << "Invalid mouse trajectory / not player holding view's turn to go" << std::endl;
    }

    // refresh window to call onPaint method
    this->Refresh();
    this->Update();
}

void BoardPanel::onMouseMove(wxMouseEvent& evt)
{
    if (this->mouseIsDown == true)
    {
        wxPoint mouseOnPanel = evt.GetPosition();
        this->mouseUp_x = mouseOnPanel.x;
        this->mouseUp_y = mouseOnPanel.y;
    }
}

void BoardPanel::onPaint(wxPaintEvent& evt)
{
    wxPaintDC dc(this);

    // create squares on board
    int color = 1;
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            if (color == 0)
            {
                dc.SetBrush(light);
            }
            else
            {
                dc.SetBrush(dark);
            }
            dc.SetPen(wxPen(wxColour(0,0,0), 1, wxPENSTYLE_TRANSPARENT));
            dc.DrawRectangle(j*(this->size/8), i*(this->size/8), this->size/8, this->size/8);
            
            color = (color + 1) % 2;
        }
        color = (color + 1) % 2;

        // draw pieces on board
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                if (this->board->getSqrPiece(this->ind2gv(i,j)) != chessboard::PIECE_NULL)
                {
                    wxBitmap resized;
                    if (this->board->getSqrOwner(this->ind2gv(i,j)) == chessboard::WHITE)
                    {
                        resized = wxBitmap( this->whitePieceImgs[this->board->getSqrPiece(this->ind2gv(i,j))].Scale(this->size/8, this->size/8) );
                    }
                    else
                    {
                        resized = wxBitmap( this->blackPieceImgs[this->board->getSqrPiece(this->ind2gv(i,j))].Scale(this->size/8, this->size/8) );
                    }
                    dc.DrawBitmap( resized, j*(this->size/8), i*(this->size/8) );
                }
            }
        }
    }

    if (activeSqr == true)
    {
        int i = this->gv2i(this->startSqr);
        int j = this->gv2j(this->startSqr);
        dc.SetBrush(wxBrush(wxColour(255,255,255), wxBRUSHSTYLE_TRANSPARENT));
        dc.SetPen(wxPen(wxColour(255,0,0), 3));
        dc.DrawRectangle(j*(this->size/8), i*(this->size/8), this->size/8, this->size/8);

        // show available squares
        std::vector<chessboard::Move> availableMoves = this->board->getValidMoves(this->startSqr);
        for (auto& mv : availableMoves)
        {
            i = this->gv2i(mv.end);
            j = this->gv2j(mv.end);
            dc.SetBrush(wxBrush(wxColour(255,255,255), wxBRUSHSTYLE_TRANSPARENT));
            dc.SetPen(wxPen(wxColour(0,0,255), 3));
            dc.DrawRectangle(j*(this->size/8), i*(this->size/8), this->size/8, this->size/8);
        }
    }
}

// panel (row,col) index to grid vector for sqr on board (depends on playerView var of board)
chessboard::GridVector BoardPanel::pos2gv(int x, int y)
{
    int i = 8*y / this->size;
    int j = 8*x / this->size;

    if (x < 0 || x > this->size || y < 0 || y > this->size)
    {
        return chessboard::GridVector(999, 999); // return invalid grid vector
    }
    if (playerView == chessboard::WHITE)
    {
        return chessboard::GridVector(j, 7-i);
    }
    else if (playerView == chessboard::BLACK)
    {
        return chessboard::GridVector(7-j, i);
    }
    else
    {
        return chessboard::GridVector(999, 999); // return invalid grid vector
    }
}

// panel (row,col) index to grid vector for sqr on board (depends on playerView var of board)
chessboard::GridVector BoardPanel::ind2gv(int i, int j)
{
    if (i < 0 || i > 7 || j < 0 || j > 7)
    {
        return chessboard::GridVector(999, 999); // return invalid grid vector
    }
    if (playerView == chessboard::WHITE)
    {
        return chessboard::GridVector(j, 7-i);
    }
    else if (playerView == chessboard::BLACK)
    {
        return chessboard::GridVector(7-j, i);
    }
    else
    {
        return chessboard::GridVector(999, 999); // return invalid grid vector
    }
}

// grid vector on board to panel row index (depends on playerView var of board)
int BoardPanel::gv2i(chessboard::GridVector gv)
{
    if (gv.rank < 0 || gv.rank > 7)
    {
        return 999;
    }
    if (playerView == chessboard::WHITE)
    {
        return 7-gv.rank;
    }
    else if (playerView == chessboard::BLACK)
    {
        return gv.rank;
    }
    else
    {
        return 999; // return invalid grid vector
    }
}

// grid vector on board to panel col index (depends on playerView var of board)
int BoardPanel::gv2j(chessboard::GridVector gv)
{
    if (gv.file < 0 || gv.file > 7)
    {
        return 999;
    }
    if (playerView == chessboard::WHITE)
    {
        return gv.file;
    }
    else if (playerView == chessboard::BLACK)
    {
        return 7-gv.file;
    }
    else
    {
        return 999; // return invalid grid vector
    }
}

/**************************************************************************************************************/
// MAIN PANEL

class MainPanel : public wxPanel
{
private:
    chessboard::Board* board;

    BoardPanel* boardPanel1;
    BoardPanel* boardPanel2;

    Comm* comm;

    int boardPadding = 20;

public:
    MainPanel(wxWindow* parent);
    ~MainPanel();

    void onResize(wxSizeEvent& evt);

    void update(int vpSizeX, int vpSizeY);
    void processMoveCallback(chessboard::MoveCallback mcb);

    DECLARE_EVENT_TABLE()
    
};

BEGIN_EVENT_TABLE(MainPanel, wxPanel)
    EVT_SIZE(MainPanel::onResize)
END_EVENT_TABLE()

MainPanel::MainPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY, wxPoint(0, 0))
{
    this->board = new chessboard::Board;

    // setup comm for communication between board panels and main panel which makes moves on board system
    // requires lambda function
    comm = new Comm;
    comm->requestMove = [&](chessboard::Move mv)
    { 
        // request move in board system
        std::cout << "Move: " << mv << std::endl;
        chessboard::MoveCallback moveCallback = this->board->requestMove(mv);
        std::cout << "Move callback: " << moveCallback << std::endl;
        this->processMoveCallback(moveCallback);

        // get game status following move
        chessboard::Status status = this->board->getStatus();
        std::cout << "Game status: " << status << std::endl;
        std::cout << "======================================================" << std::endl;
    };

    // initialise board panels with comm
    this->boardPanel1 = new BoardPanel(this, board, chessboard::WHITE, comm);
    this->boardPanel2 = new BoardPanel(this, board, chessboard::BLACK, comm);

    // setup board
    this->board->setup();
}

MainPanel::~MainPanel()
{

}

void MainPanel::processMoveCallback(chessboard::MoveCallback mcb)
{
    if (mcb == chessboard::SUCCESS)
    {
        // step both panels
        this->boardPanel1->step();
        this->boardPanel2->step();
    }
    else if (mcb == chessboard::FAILURE)
    {
        std::cout << "Move failed" << std::endl;
    }
}

// event handler for resizing of main frame
void MainPanel::onResize(wxSizeEvent& evt)
{
    this->update(evt.GetSize().GetX(), evt.GetSize().GetY());
}

// refreshes the main panel and its children after an event
void MainPanel::update(int vpSizeX, int vpSizeY)
{
    // refresh size and position of board panels
    int boardBoxWidth = (vpSizeX / 2) - 2*boardPadding;
    int boardBoxHeight = (vpSizeY * 0.6) - 2*boardPadding;
    int boardSize, surplus, x, y;
    
    if (boardBoxWidth > boardBoxHeight)
    {
        // width > height
        boardSize = boardBoxHeight;
        surplus = boardBoxWidth - boardBoxHeight;
        x = boardPadding + surplus / 2;
        y = boardPadding;
        
    }
    else
    {
        // width <= height
        boardSize = boardBoxWidth;
        surplus = boardBoxHeight - boardBoxWidth;
        x = boardPadding;
        y = boardPadding + surplus / 2;
    }

    this->boardPanel1->setPanelDims(x, y, boardSize);
    this->boardPanel2->setPanelDims(x + vpSizeX / 2, y, boardSize);
}

/**************************************************************************************************************/
// MAIN FRAME

class MainFrame : public wxFrame
{
private:

public:
    
    MainFrame();
    ~MainFrame();

    MainPanel* mainPanel;

};

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, "Chess", wxPoint(30, 30), wxSize(800, 600))
{
    this->mainPanel = new MainPanel(this);
    this->SetMinSize(wxSize(700, 600));
}

MainFrame::~MainFrame()
{

}

/**************************************************************************************************************/
// APP CLASS

class App : public wxApp
{
private:
    MainFrame* m_frame = nullptr;

public:
    App();
    ~App();

    virtual bool OnInit();
};

wxIMPLEMENT_APP(App); // main fcn

App::App()
{

}

App::~App()
{

}

bool App::OnInit()
{
    wxInitAllImageHandlers();
    this->m_frame = new MainFrame();
    this->m_frame->Show();

    return true;
}