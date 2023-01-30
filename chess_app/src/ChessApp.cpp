#include "../../chessboard/chessboard.h"
#include <wx/wx.h>
#include <thread>
#include <functional>

struct Comm
{
    std::function<void(chessboard::Move)> requestMove;
};

/**************************************************************************************************************/
// CHESS BOARD

class BoardPanel : public wxPanel
{
private:
    int pos_x, pos_y;
    int size;

    Comm* comm;

    chessboard::Board* board;
    chessboard::Player playerView;

    wxColor light = wxColor(100, 100, 100);
    wxColor dark = wxColor(200, 200, 200);

    std::unordered_map<chessboard::Piece, wxImage> whitePieceImgs;
    std::unordered_map<chessboard::Piece, wxImage> blackPieceImgs;

    bool mouseIsDown = false;
    bool activeSqr = false;
    int mouseDown_x, mouseDown_y;
    int mouseUp_x, mouseUp_y;
    chessboard::GridVector gvDown, gvUp, startSqr, endSqr;
    
    chessboard::GridVector pos2gv(int x, int y);
    chessboard::GridVector ind2gv(int x, int y);

public:
    BoardPanel(wxWindow* parent, chessboard::Board* board, chessboard::Player playerView, Comm* comm);
    ~BoardPanel();

    void step();
    void setPanelDims(int x, int y, int width);
    void render(wxPaintDC& dc);

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
            // clicked on the square
            if (this->activeSqr == false)
            {
                // no current active square, activate this square
                this->startSqr = this->gvUp;
                this->activeSqr = true;
                std::cout << "Activated " << this->gvUp << std::endl;
            }
            else
            {
                // square already active, check that it is not same square
                if (this->gvUp == this->startSqr)
                {
                    // cancelling move
                    std::cout << "Cancelled move" << std::endl;
                }
                else
                {
                    this->endSqr = this->gvUp;
                    std::cout << "(Select) Requested move: " << this->startSqr << ", " << this->endSqr << std::endl;
                    if (this->board->getSqrOwner(this->startSqr) == this->playerView)
                    {
                        //chessboard::MoveCallback moveCallback = this->board->move({this->startSqr, this->endSqr});
                        //std::cout << "Move callback: " << moveCallback << std::endl;
                        this->comm->requestMove(chessboard::Move(this->startSqr, this->endSqr));
                    }
                    else
                    {
                        std::cout << "Square not owned by player to view" << std::endl;
                    }
                }
                this->activeSqr = false;
            }
        }
        else
        {
            // dragged piece from one square to another
            this->startSqr = this->gvDown;
            this->endSqr = this->gvUp;
            std::cout << "(Drag) Requested move: " << this->startSqr << ", " << this->endSqr << std::endl;
            if (this->board->getSqrOwner(this->startSqr) == this->playerView)
            {
                //chessboard::MoveCallback moveCallback = this->board->move({this->startSqr, this->endSqr});
                //std::cout << "Move callback: " << moveCallback << std::endl;
                this->comm->requestMove(chessboard::Move(this->startSqr, this->endSqr));
            }
            else
            {
                std::cout << "Square not owned by player to view" << std::endl;
            }
        }
    }
    else
    {
        // mouse trajectory not valid
        std::cout << "Invalid mouse trajectory" << std::endl;
    }
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
    this->render(dc);
}

// draws the board and pieces where they currently stand
void BoardPanel::render(wxPaintDC& dc)
{
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
            dc.SetPen(wxPen(wxColor(0,0,0), 0));
            
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
}

// returns position within panel to grid vector for sqr
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

// returns position within panel to grid vector for sqr
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

/**************************************************************************************************************/
// MAIN PANEL

class MainPanel : public wxPanel
{
private:
    chessboard::Board* board;

    BoardPanel* boardPanel1;
    BoardPanel* boardPanel2;

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
    Comm* comm = new Comm;
    comm->requestMove = [&](chessboard::Move mv)
    { 
        std::cout << "Move: " << mv << std::endl;
        chessboard::MoveCallback moveCallback = this->board->move(mv);
        std::cout << "Move callback: " << moveCallback << std::endl;
        this->processMoveCallback(moveCallback);
    };

    // initialise board panels
    this->boardPanel1 = new BoardPanel(this, board, chessboard::WHITE, comm);
    this->boardPanel2 = new BoardPanel(this, board, chessboard::BLACK, comm);

    this->board->setup();
}

MainPanel::~MainPanel()
{

}

void MainPanel::processMoveCallback(chessboard::MoveCallback mcb)
{
    if (mcb == chessboard::SUCCESS)
    {
        this->boardPanel1->step();
        this->boardPanel2->step();
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