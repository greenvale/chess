
#include <iostream>
#include "chess_consol.hpp"

int main()
{
    ChessConsol chessConsol;

    std::vector<std::string> preMoves1 = {
        "e2 e3",
        "e7 e5",
        "e3 e4",
        "f7 f5",
        "a2 a3",
        "g8 f6",
        "d1 h5"
    };

    chessConsol.launch(preMoves);

}