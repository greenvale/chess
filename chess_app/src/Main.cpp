#include "Main.h"

Main::Main() : wxFrame(nullptr, wxID_ANY, "Chess", wxPoint(30, 30), wxSize(1500, 800))
{
    // create grids for black and white (same pc mode)
    grid[WHITE] = new wxButton*[64];
    grid[BLACK] = new wxButton*[64];

    wxFont font(24, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false);

    // create buttons
    for (int i = 0; i < 8; ++i) // iterate through file on grid (not board)
        for (int j = 0; j < 8; ++j) // iterate through rank on grid (not board)
        {
            grid[WHITE][j*8 + i] = new wxButton(this, 10000 + (j*8 + i), "", wxPoint(50 + 50*i,     50 + 50*j), wxSize(50, 50));
            grid[BLACK][j*8 + i] = new wxButton(this, 10100 + (j*8 + i), "", wxPoint(850 + 50*i,    50 + 50*j), wxSize(50, 50));

            grid[WHITE][j*8 + i]->SetFont(font);
            grid[BLACK][j*8 + i]->SetFont(font);

            grid[WHITE][j*8 + i]->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Main::OnWhiteGridClicked, this);
            grid[BLACK][j*8 + i]->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Main::OnBlackGridClicked, this);
        }

    // create dictionary of piece tags
    pieceTag[PAWN] = 'p';
    pieceTag[ROOK] = 'r';
    pieceTag[KNIGHT] = 'n';
    pieceTag[BISHOP] = 'b';
    pieceTag[QUEEN] = 'q';
    pieceTag[KING] = 'k';

    // create chessboard backend
    board = new Board;
    board->setup();
    activeSqr = GridVector(-1,-1);

    // update grids
    updateGrids();
}

Main::~Main()
{
    delete[] grid[WHITE], grid[BLACK];
}

// square on White's grid clicked
void Main::OnWhiteGridClicked(wxCommandEvent& evt)
{
    int file = ((evt.GetId() - 10000) % 8);
    int rank = 7 - ((evt.GetId() - 10000) / 8);

    execute(GridVector(file, rank));
    updateGrids();

    evt.Skip(); // allows other events to pick up this click
}

// square on Black's grid clicked
void Main::OnBlackGridClicked(wxCommandEvent& evt)
{
    int file = 7 - ((evt.GetId() - 10100) % 8);
    int rank = ((evt.GetId() - 10100) / 8);

    execute(GridVector(file, rank));
    updateGrids();

    evt.Skip(); // allows other events to pick up this click
}

void Main::execute(GridVector sqr)
{
    if (activeSqr == GridVector(-1, -1))
    {
        // activate square
        activeSqr = GridVector(sqr);
    }
    else
    {
        if (activeSqr == sqr)
            // move cancelled 
            activeSqr = GridVector(-1, -1);
        else 
        {
            // make move on board
            MoveCallback success = board->move({activeSqr, sqr});
            if (success != SUCCESS)
                std::cout << "Invalid move" << std::endl;
            
            activeSqr = GridVector(-1, -1);

            // check on game status
            if (board->getStatus() == IN_PROGRESS)
            {
                std::cout << "Game in progress" << std::endl;
            }
            else
            {
                std::cout << "Game outcome: " << board->getStatus() << std::endl;
            }
        }
    }
}

void Main::updateGrids()
{
    // display board
    int colour = 0;
    for (int i = 0; i < 8; ++i) // iterate through file on board (not grid)
    {
        for (int j = 0; j < 8; ++j) // iterate through rank on board (not grid)
        {
            if (colour == 0)
            {
                grid[WHITE][j*8 + i]->SetBackgroundColour(wxColor(200, 200, 220));
                grid[BLACK][j*8 + i]->SetBackgroundColour(wxColor(200, 200, 220));
            }
            else
            {
                grid[WHITE][j*8 + i]->SetBackgroundColour(wxColor(100, 100, 200));
                grid[BLACK][j*8 + i]->SetBackgroundColour(wxColor(100, 100, 200));
            }
                    
            colour = (colour + 1) % 2;

            if (board->getSqrPiece({i,j}) != PIECE_NULL)
            {
                if (board->getSqrOwner({i,j}) == WHITE)
                {
                    grid[WHITE][(7-j)*8 + (i)]->SetLabel((char) toupper( pieceTag[board->getSqrPiece({i,j})] ));
                    grid[BLACK][(j)*8 + (7-i)]->SetLabel((char) toupper( pieceTag[board->getSqrPiece({i,j})] ));
                }
                else
                {
                    grid[WHITE][(7-j)*8 + (i)]->SetLabel( pieceTag[board->getSqrPiece({i,j})] );
                    grid[BLACK][(j)*8 + (7-i)]->SetLabel( pieceTag[board->getSqrPiece({i,j})] );
                }
            }
            else
            {
                grid[WHITE][(7-j)*8 + (i)]->SetLabel("");
                grid[BLACK][(j)*8 + (7-i)]->SetLabel("");
            }
        }
        colour = (colour + 1) % 2;
    }

    // disable all buttons initially
    for (int i = 0; i < 8; ++i) // iterate through file on board (not grid)
        for (int j = 0; j < 8; ++j) // iterate through rank on board (not grid)
        {
            grid[WHITE][j*8 + i]->Enable(false);
            grid[BLACK][j*8 + i]->Enable(false);
        }

    // get player to move
    Player ptm = board->getPlayerToMove();

    if (activeSqr == GridVector(-1, -1))
    {
        // enable pieces that can be moved
        for (int i = 0; i < 8; ++i) // iterate through file on board (not grid)
            for (int j = 0; j < 8; ++j) // iterate through rank on board (not grid)
            {
                if (board->getSqrOwner({i,j}) == ptm)
                {
                    if (ptm == WHITE)
                        grid[WHITE][(7-j)*8 + (i)]->Enable(true);
                    else
                        grid[BLACK][(j)*8 + (7-i)]->Enable(true);
                }
            }
    }
    else
    {
        // enable squares covered by selected piece
        std::vector<GridVector> options = board->getPieceMoveOptions(activeSqr);
        for (auto& sqr : options)
        {
            if (ptm == WHITE)
                grid[WHITE][(7-sqr.rank)*8 + (sqr.file)]->Enable(true);
            else
                grid[BLACK][(sqr.rank)*8 + (7-sqr.file)]->Enable(true);
        }
        // allow selected piece to be pressed to cancel move
        if (ptm == WHITE)
            grid[WHITE][(7-activeSqr.rank)*8 + (activeSqr.file)]->Enable(true);
        else
            grid[BLACK][(activeSqr.rank)*8 + (7-activeSqr.file)]->Enable(true);
    }
}