#include "../../chessboard/chessboard.h"
#include <wx/wx.h>
#include <wx/vscroll.h>


/**************************************************************************************************************/
// CHESS BOARD

class BoardPanel : public wxPanel
{
private:
    int pos_x, pos_y;
    int size;

    chessboard::Board* board;
    chessboard::Player playerView;
    chessboard::Move* moveRequ = nullptr;

    wxPanel** sqrs;
    int sqrIdRoot;

    wxColor light = wxColor(100, 100, 100);
    wxColor dark = wxColor(200, 200, 200);

    bool mouseIsDown = false;
    bool activeSqr = false;
    int mouseDown_x, mouseDown_y;
    int mouseUp_x, mouseUp_y;
    chessboard::GridVector gvDown, gvUp, startSqr, endSqr;
    
    chessboard::GridVector pos2gv(int x, int y);

public:
    BoardPanel(wxWindow* parent, int sqrIdRoot);
    ~BoardPanel();

    void setPanelDims(int x, int y, int width);
    void refresh();

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

BoardPanel::BoardPanel(wxWindow* parent, int sqrIdRoot) : wxPanel(parent, wxID_ANY, wxPoint(0, 0), wxSize(0, 0))
{
    this->playerView = chessboard::WHITE;
}

BoardPanel::~BoardPanel()
{

}

void BoardPanel::setPanelDims(int x, int y, int size)
{
    this->pos_x = x;
    this->pos_y = y;
    this->size = size;
}

void BoardPanel::refresh()
{
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

    if (this->gvUp != chessboard::GridVector(999,999))
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
            //dc.SetPen(wxPen(wxColor(200,100,100), 10));
            
            dc.DrawRectangle(j*(this->size/8), i*(this->size/8), this->size/8, this->size/8);
            
            color = (color + 1) % 2;
        }
        color = (color + 1) % 2;
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
        return chessboard::GridVector(7-i, j);
    }
    else if (playerView == chessboard::BLACK)
    {
        return chessboard::GridVector(i, 7-j);
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

    BoardPanel* boardPanel1;
    BoardPanel* boardPanel2;

    int boardPadding = 20;

public:
    MainPanel(wxWindow* parent);
    ~MainPanel();

    void onResize(wxSizeEvent& evt);

    void refresh(int vpSizeX, int vpSizeY);
    
};

MainPanel::MainPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY, wxPoint(0, 0))
{
    this->Bind(wxEVT_SIZE, &MainPanel::onResize, this);

    boardPanel1 = new BoardPanel(this, 10000);
    boardPanel2 = new BoardPanel(this, 10100);
}

MainPanel::~MainPanel()
{

}

// event handler for resizing of main frame
void MainPanel::onResize(wxSizeEvent& evt)
{
    this->refresh(evt.GetSize().GetX(), evt.GetSize().GetY());
}

// refreshes the main panel and its children after an event
void MainPanel::refresh(int vpSizeX, int vpSizeY)
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

    boardPanel1->setPanelDims(x, y, boardSize);
    boardPanel2->setPanelDims(x + vpSizeX / 2, y, boardSize);

    boardPanel1->refresh();
    boardPanel2->refresh();

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
    this->m_frame = new MainFrame();
    this->m_frame->Show();

    return true;
}