/* Chess library */
#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cstring>
#include <algorithm>

/**************************************************************************************/
// GRIDVECTOR STRUCT

struct GridVector
{
    int file, rank;

    GridVector() {}
    GridVector(const int& file, const int& rank) : file(file), rank(rank) {}
};

GridVector operator+(GridVector lh, GridVector rh)
{
    return GridVector(lh.file + rh.file, lh.rank + rh.rank);
}
GridVector operator*(GridVector lh, double rh)
{
    return GridVector(lh.file * rh, lh.rank * rh);
}
GridVector operator*(double lh, GridVector rh)
{
    return GridVector(lh * rh.file, lh * rh.rank);
}
std::ostream& operator<<(std::ostream& os, GridVector gv)
{
    os << "(" << gv.file << ", " << gv.rank << ")";
    return os;
}
bool operator==(GridVector lh, GridVector rh)
{
    return ((lh.file == rh.file) && (lh.rank == rh.rank));
}

/**************************************************************************************/
// PIECE ENUM 

enum Piece {
    PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING, PIECE_NULL
};

std::ostream& operator<<(std::ostream& os, Piece t)
{
    switch(t)
    {
        case PAWN: os << "Pawn"; break;
        case ROOK: os << "Rook"; break;
        case KNIGHT: os << "Knight"; break;
        case BISHOP: os << "Bishop"; break;
        case QUEEN: os << "Queen"; break;
        case KING: os << "King"; break;
    }
    return os;
}

/**************************************************************************************/
// PLAYER ENUM 

enum Player {
    WHITE, BLACK, PLAYER_NULL
};

std::ostream& operator<<(std::ostream& os, Player t)
{
    switch(t)
    {
        case WHITE: os << "White"; break;
        case BLACK: os << "Black"; break;
        case PLAYER_NULL: os << "Player NULL"; break;
    }
    return os; 
}

Player operator!(Player p)
{
    if (p == WHITE)
    {
        return BLACK;
    }
    else if (p == BLACK)
    {
        return WHITE;
    }
    else
    {
        return PLAYER_NULL;
    }
}

/**************************************************************************************/
// STATUS ENUM 

enum Status
{
    IN_PROGRESS, WHITE_WIN, BLACK_WIN, DRAW, STALEMATE
};

/**************************************************************************************/

struct Move
{
    GridVector start; 
    GridVector end;

    Move() {}
    Move(GridVector start, GridVector end) : start(start), end(end) {}
};

std::ostream& operator<<(std::ostream& os, Move mv)
{
    os << "(" << mv.start.file << ", " << mv.start.rank << ") -> (" << mv.end.file << ", " << mv.end.rank << ")";
    return os;
}

/**************************************************************************************/

struct SqrCover
{
    GridVector origin;
    Piece piece;
    Player owner;

    SqrCover() {}
    SqrCover(GridVector origin, Piece piece, Player owner) : origin(origin), piece(piece), owner(owner) {}
};

/**************************************************************************************/


/**************************************************************************************/

class Board
{

private:

    std::vector<Piece> sqrPieces;
    std::vector<Player> sqrOwners;

    // Static rules/assets
    std::unordered_map<Piece, std::string> whitePieceTags; // to be changed - delegated to consol
    std::unordered_map<Piece, std::string> blackPieceTags; // to be changed - delegated to consol
    std::unordered_map<Piece, std::vector<GridVector>> moveOptions;
    std::unordered_map<Piece, bool> moveConstraint;

    // Functionality assets
    std::vector<std::vector<SqrCover>> sqrCoverage;

    Player playerToMove;
    Status status;
    Player check;
    std::unordered_map<Player, GridVector> kingPos;
    std::vector<GridVector> pinnedSqrs;
    std::vector<GridVector> checkRaySqrs;
    std::vector<Move> checkEscapes;
    
    std::unordered_map<Player, std::vector<GridVector>> doublePushedPawn;

public:

    Board();
    void setup();

    bool move(Move pieceMove);
    void step();


    
    void display(); // to be changed - delegated to consol

private:

    int ind(GridVector sqr);

    Piece getSqrPiece(GridVector sqr);
    Player getSqrOwner(GridVector sqr);

    bool emptySqr(GridVector sqr);
    bool validSqr(GridVector sqr);
    void setSqr(GridVector sqr, Piece type, Player owner);
    void clearSqr(GridVector sqr);

    void execute(Move pieceMove);

    void updateSqrCoverage();
    std::vector<GridVector> getRay(GridVector origin, GridVector dir);
    void updateCheckEscapes();

    bool validateMove(Move pieceMove);

};

// ctor for board
Board::Board()
{

    // INTIALISE PIECE SETTINGS
    // Pawn
    whitePieceTags[PAWN] = "P";
    blackPieceTags[PAWN] = "p";
    moveOptions[PAWN] = {}; // pawn move options are defined in function
    moveConstraint[PAWN] = true;

    // Rook
    whitePieceTags[ROOK] = "R";
    blackPieceTags[ROOK] = "r";
    moveOptions[ROOK] = {
        {0,1},
        {0,-1},
        {1,0},
        {-1,0},
    };
    moveConstraint[ROOK] = false;

    // Knight
    whitePieceTags[KNIGHT] = "N";
    blackPieceTags[KNIGHT] = "n";
    moveOptions[KNIGHT] = {
        {-2,-1},
        {-1,-2},
        {2,-1},
        {-1,2},
        {-2,1},
        {1,-2},
        {2,1},
        {1,2},
    };
    moveConstraint[KNIGHT] = false;

    // Bishop
    whitePieceTags[BISHOP] = "B";
    blackPieceTags[BISHOP] = "b";
    moveOptions[BISHOP] = {
        {-1,-1},
        {1,-1},
        {-1,1},
        {1,1},
    };
    moveConstraint[BISHOP] = false;

    // Queen
    whitePieceTags[QUEEN] = "Q";
    blackPieceTags[QUEEN] = "q";
    moveOptions[QUEEN] = {
        {0,1},
        {0,-1},
        {1,0},
        {-1,0},
        {-1,-1},
        {1,-1},
        {-1,1},
        {1,1},
    };
    moveConstraint[QUEEN] = false;

    // King
    whitePieceTags[KING] = "K";
    blackPieceTags[KING] = "k";
    moveOptions[KING] = {
        {0,1},
        {0,-1},
        {1,0},
        {-1,0},
        {-1,-1},
        {1,-1},
        {-1,1},
        {1,1},
    };
    moveConstraint[KING] = true;

    // Initialise empty board
    sqrPieces = std::vector<Piece>(64, PIECE_NULL);
    sqrOwners = std::vector<Player>(64, PLAYER_NULL);

    // functionality assets
    // create covereage vectors for each square
    sqrCoverage = std::vector<std::vector<SqrCover>>(64, std::vector<SqrCover>());

}

void Board::setup()
{
    for (int i = 0; i < 8; ++i)
    {
        setSqr({i,1}, PAWN, WHITE);
        setSqr({i,6}, PAWN, BLACK);
    }
    setSqr({0,0}, ROOK, WHITE);
    setSqr({7,0}, ROOK, WHITE);
    setSqr({0,7}, ROOK, BLACK);
    setSqr({7,7}, ROOK, BLACK);

    setSqr({1,0}, KNIGHT, WHITE);
    setSqr({6,0}, KNIGHT, WHITE);
    setSqr({1,7}, KNIGHT, BLACK);
    setSqr({6,7}, KNIGHT, BLACK);

    setSqr({2,0}, BISHOP, WHITE);
    setSqr({5,0}, BISHOP, WHITE);
    setSqr({2,7}, BISHOP, BLACK);
    setSqr({5,7}, BISHOP, BLACK);

    setSqr({3,0}, QUEEN, WHITE);
    setSqr({3,7}, QUEEN, BLACK);

    setSqr({4,0}, KING, WHITE);
    setSqr({4,7}, KING, BLACK);

    kingPos[WHITE] = GridVector({4,0});
    kingPos[BLACK] = GridVector({4,7});

    playerToMove = WHITE;
    status = IN_PROGRESS;
    check = PLAYER_NULL;
}

/* ======================================== Basic internal operations ======================================== */

int Board::ind(GridVector sqr)
{
    return sqr.rank*8 + sqr.file;
}

Piece Board::getSqrPiece(GridVector sqr)
{
    return sqrPieces[ind(sqr)];
}

Player Board::getSqrOwner(GridVector sqr)
{
    return sqrOwners[ind(sqr)];
}

bool Board::emptySqr(GridVector sqr)
{
    return (sqrPieces[ind(sqr)] == PIECE_NULL);
}

bool Board::validSqr(GridVector sqr)
{
    return ((sqr.file >= 0) && (sqr.file < 8) && (sqr.rank >= 0) && (sqr.rank < 8));
}

void Board::setSqr(GridVector sqr, Piece type, Player owner)
{
    sqrPieces[ind(sqr)] = type;
    sqrOwners[ind(sqr)] = owner;
}

void Board::clearSqr(GridVector sqr)
{
    sqrPieces[ind(sqr)] = PIECE_NULL;
    sqrOwners[ind(sqr)] = PLAYER_NULL;
}

void Board::execute(Move pieceMove)
{
    Piece piece = sqrPieces[ind(pieceMove.start)];
    Player owner = sqrOwners[ind(pieceMove.start)];
    clearSqr(pieceMove.start);
    sqrPieces[ind(pieceMove.end)] = piece;
    sqrOwners[ind(pieceMove.end)] = owner;

    // keep track of the king positions
    if (pieceMove.start == kingPos[playerToMove])
    {
        kingPos[playerToMove] = pieceMove.end;
    }
}

/* ======================================== Board analysis and rule enforcement ======================================== */

// step the board system to update status for both players' pieces
void Board::step()
{
    // clear data
    for (int i = 0; i < 64; ++i)
    {
        sqrCoverage[i].clear(); // refresh the coverage vectors for each square to update square coverage
    }
    check = PLAYER_NULL;
    checkRaySqrs.clear();    
    pinnedSqrs.clear();
    checkEscapes.clear();

    updateSqrCoverage();        // firstly, update the square coverage after a move to assess current position on board

    std::cout << "Player in check: " << check << std::endl;
    for (auto& sqr : pinnedSqrs)
    {
        std::cout << "Pinned sqr: " << sqr << std::endl;
    }
    for (auto& sqr : checkRaySqrs)
    {
        std::cout << "Check ray sqr: " << sqr << std::endl;
    }

    if (check != PLAYER_NULL)
    {
        updateCheckEscapes();
    }
}

// returns ray
std::vector<GridVector> Board::getRay(GridVector origin, GridVector dir)
{
    std::vector<GridVector> ray = {};
    bool stop = false;
    int step = 1;
    while (stop == false)
    {
        if (validSqr(origin + dir*step) == true)
        {
            ray.push_back(origin + dir*step);
            step++;
        }
        else
        {
            stop = true;
        }
    }
    return ray;
}

// updates the coverage on each square by each players' pieces
void Board::updateSqrCoverage()
{
    // iterate through each square of the board
    for (int i = 0; i < 8; ++i) // file
    {
        for (int j = 0; j < 8; ++j) // rank
        {
            if (!emptySqr({i,j})) // if the square in question is not empty, then examine the piece on the square
            {
                Piece piece = getSqrPiece({i,j});
                Player owner = getSqrOwner({i,j});

                if (moveConstraint[piece] == true) // MOVE CONSTRAINED PIECE (Pawn, Knight, King)
                {
                    // PAWN
                    if (piece == PAWN)
                    {
                        // get sign of pawn moves depending on player
                        int sign = 1;
                        if (owner == BLACK)
                            sign = -1;
                        
                        // pawn push
                        if ((validSqr(GridVector(i,j + sign))) && (getSqrOwner(GridVector(i,j + sign)) == PLAYER_NULL))
                        {
                            sqrCoverage[ind(GridVector(i,j + sign))].push_back({ {i,j}, piece, owner });

                            // double pawn push
                            if ((((owner == WHITE) && (j == 1)) || ((owner == BLACK) && (j == 6))) && (validSqr(GridVector(i,j + 2*sign))) && (getSqrOwner(GridVector(i,j + 2*sign)) == PLAYER_NULL))
                            {
                                sqrCoverage[ind(GridVector(i,j + 2*sign))].push_back({ {i,j}, piece, owner });
                            }
                        }

                        // pawn capture
                        if ((validSqr(GridVector(i - 1,j + sign)) && (getSqrOwner(GridVector(i - 1,j + sign)) == !owner)))
                        {
                            sqrCoverage[ind(GridVector(i - 1,j + sign))].push_back({ {i,j}, piece, owner });
                        }
                        if ((validSqr(GridVector(i + 1,j + sign))) && (getSqrOwner(GridVector(i + 1,j + sign)) == !owner))
                        {
                            sqrCoverage[ind(GridVector(i + 1,j + sign))].push_back({ {i,j}, piece, owner });
                        }
                    }
                    // KNIGHT / KING
                    else
                    {
                        for (auto& mv : moveOptions[piece])
                        {
                            if ((validSqr(GridVector(i,j) + mv) == true) && (getSqrOwner(GridVector(i,j) + mv) != owner))
                            {
                                sqrCoverage[ind(GridVector(i,j) + mv)].push_back({ {i,j}, piece, owner }); // the piece covers this square, add to coverage
                            }
                        }
                    }
                }
                else // MOVE UNCONSTRAINED PIECE (Rook, Bishop, Queen)
                {
                    for (auto& mv : moveOptions[piece])
                    {
                        std::vector<GridVector> ray = getRay({i,j}, mv);

                        std::vector<int> friendsInRay = {};
                        std::vector<int> enemiesInRay = {};
                        int enemyKingInRay = -1;

                        // determine steps at which different pieces lie (friend pieces, enemy piecees, enemy king)
                        bool stop = false;
                        for (int k = 0; k < ray.size(); ++k)
                        {
                            if (getSqrOwner(ray[k]) == owner) // encountered friendly in ray
                            {
                                friendsInRay.push_back(k);
                            }
                            else if ((getSqrPiece(ray[k]) != KING) && (getSqrOwner(ray[k]) == !owner)) // encountered enemy but not king in ray
                            {
                                enemiesInRay.push_back(k);
                            }
                            else if ((getSqrPiece(ray[k]) == KING) && (getSqrOwner(ray[k]) == !owner)) // encountered king in ray
                            {
                                enemyKingInRay = k;
                                stop = true;
                            }

                            if ((friendsInRay.size() == 0) && (enemiesInRay.size() < 2)) // if no friend encountered and is blocking the piece, add cover
                                sqrCoverage[ind(ray[k])].push_back({ {i,j}, piece, owner });

                            if (stop == true) // if enemy king encountered, stop going through ray, no other information is required
                                break;
                        }

                        if ((enemyKingInRay > -1) && (friendsInRay.size() == 0) && (enemiesInRay.size() == 0))
                        {
                            // checking king
                            if (check != owner)
                                check = !owner;
                            else
                                std::cout << "CATASTROPHIC ERROR - BOTH KINGS HAVE BECOME CHECKED!" << std::endl;


                            // include check ray squares
                            for (int k = 0; k < enemyKingInRay; ++k)
                            {
                                checkRaySqrs.push_back(ray[k]);
                            }
                        }
                        else if ((enemyKingInRay > -1) && (friendsInRay.size() == 0) && (enemiesInRay.size() == 1) && (enemiesInRay[0] < enemyKingInRay))
                        {
                            // pinned this piece
                            pinnedSqrs.push_back(ray[enemiesInRay[0]]);
                        }
                    }
                }
            }
        }
    }
    
    /*
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            std::cout << "Square: " << GridVector(i,j) << ": ";
            for (auto& sqrs : sqrCoverage[ind({i,j})])
            {
                std::cout << sqrs.origin << " - " << sqrs.piece << " - " << sqrs.owner << "; ";
            }
            std::cout << std::endl;
        }
    }
    
    */
}

// get check escapes (assuming a player is in check)
void Board::updateCheckEscapes()
{
    // get number of pieces checking the king
    int numCheckers = 0;
    for (auto& cv : sqrCoverage[ind(kingPos[check])])
    {
        std::cout << "Piece origin: " << cv.origin << std::endl;
        if (cv.owner = !check)
            numCheckers++;
    }

    std::cout << "Num checkers: " << numCheckers << std::endl;
}  

// validates move by checking if the piece at move.start covers move.end
// ensures that the piece in question isn't pinned
// if the player is in check, this move must move them out of check, by: 
//  1. moving the king to a square with no opponent cover, or
//  2. removing the only opponent's one and only piece that covers the king's square (not available if two pieces cover the king's square)
//  3. blocking the check of the opponent's one and only piece checking the king
bool Board::validateMove(Move pieceMove)
{
    // ensure that both squares provided in move are valid squares on the board and are indexed properly
    if ( (!(validSqr(pieceMove.start))) || (!(validSqr(pieceMove.end))) )
        return false;

    // ensure that the piece being moved belongs to the player to move
    if (sqrOwners[ind(pieceMove.start)] != playerToMove)
        return false;

    // check if owner is in check
    
    // check if piece is pinned

    // check that this piece covers this square
    for (auto& cvr : sqrCoverage[ind(pieceMove.end)])
    {
        if (cvr.origin == pieceMove.start)
        {
            return true;
        }
    }
    return false;
}

// public function for moving pieces from outside consol
bool Board::move(Move pieceMove)
{
    if (validateMove(pieceMove) == true)
    {
        execute(pieceMove);

        playerToMove = !playerToMove;

        return true;
    }
    else
    {
        return false;
    }
}

/* ======================================== Board display in console ======================================== */

void Board::display()
{
    int colour = 0;
    for (int i = 7; i >= 0; --i)
    {
        std::cout << " " << (i+1) << "   ";
        for (int j = 0; j < 8; ++j)
        {
            if (sqrOwners[ind({j,i})] == PLAYER_NULL)
            {
                if (colour == 0)
                    std::cout << " ";
                else 
                    std::cout << "#";
            }
            else 
            {
                if (sqrOwners[ind({j,i})] == WHITE)
                {
                    std::cout << whitePieceTags[ sqrPieces[ind({j,i})] ];
                }
                else
                {
                    std::cout << blackPieceTags[ sqrPieces[ind({j,i})] ];
                }
            }
            colour = (colour + 1) % 2;
            std::cout << " ";
        }
        std::cout << std::endl;
        colour = (colour + 1) % 2;
    }
    std::cout << std::endl;
    std::cout << "     ";
    for (int i = 0; i < 8; ++i)
        std::cout << static_cast<char>('a'+i) << " ";
    std::cout << std::endl;
}