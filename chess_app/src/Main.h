#pragma once

#include <wx/wx.h>
#include "../../chessboard/chessboard.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>

class Main : public wxFrame
{
public:
    Main();
    ~Main();

public:

    std::unordered_map<Player, wxButton**> grid;

public:

    Board* board;

    std::unordered_map<Piece, char> pieceTag;

    GridVector activeSqr;

    void OnWhiteGridClicked(wxCommandEvent& evt);
    void OnBlackGridClicked(wxCommandEvent& evt);

    void execute(GridVector sqr);
    void updateGrids();

};