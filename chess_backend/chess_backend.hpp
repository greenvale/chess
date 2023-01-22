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
        case PAWN:      os << "Pawn";   break;
        case ROOK:      os << "Rook";   break;
        case KNIGHT:    os << "Knight"; break;
        case BISHOP:    os << "Bishop"; break;
        case QUEEN:     os << "Queen";  break;
        case KING:      os << "King";   break;
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
        case WHITE:         os << "White";          break;
        case BLACK:         os << "Black";          break;
        case PLAYER_NULL:   os << "Player NULL";    break;
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
    IN_PROGRESS, CHECKMATE, DRAW, STALEMATE
};

/**************************************************************************************/
// MOVE STRUCT

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
bool operator==(Move lh, Move rh)
{
    return ((lh.start.file == rh.start.file) && (lh.start.rank == rh.start.rank) && (lh.end.file == rh.end.file) && (lh.end.rank == rh.end.rank));
}

/**************************************************************************************/
// SQUARE COVER STRUCT

struct SqrCover
{
    GridVector origin;
    Piece piece;
    Player owner;

    SqrCover() {}
    SqrCover(GridVector origin, Piece piece, Player owner) : origin(origin), piece(piece), owner(owner) {}
};

/**************************************************************************************/
// MOVE CALLBACK ENUM

enum MoveCallback
{
    SUCCESS, GAME_OVER, INVALID_SQR, CHECK_NOT_ESCAPED, PINNED_PIECE, ENEMY_PIECE, ILLEGAL_TRAJ, INVALID_INPUT
};
std::ostream& operator<<(std::ostream& os, MoveCallback t)
{
    switch(t)
    {
        case SUCCESS:               os << "Success";                            break;
        case GAME_OVER:             os << "Game over";                          break;
        case INVALID_SQR:           os << "Invalid square";                     break;
        case CHECK_NOT_ESCAPED:     os << "Check not escaped";                  break;
        case PINNED_PIECE:          os << "Piece pinned";                       break;
        case ENEMY_PIECE:           os << "Enemy piece, cannot move";           break;
        case ILLEGAL_TRAJ:          os << "Trajectory illegal for this piece";  break;
        case INVALID_INPUT:         os << "Invalid input";                      break;

    }
    return os; 
}

/**************************************************************************************/
// BOARD CLASS

class Board
{

private:

    std::vector<Piece> sqrPieces;
    std::vector<Player> sqrOwners;

    // Static rules/assets
    std::unordered_map<Piece, std::vector<GridVector>> moveTrajs;
    std::unordered_map<Piece, bool> moveConstraint;

    // Functionality assets & flags
    Player playerToMove;
    Player winner;
    Status status;
    Player check;
    std::vector<std::vector<SqrCover>> sqrCoverage;
    std::unordered_map<Player, GridVector> kingSqr;
    std::vector<GridVector> pinnedSqrs;
    std::vector<GridVector> checkRaySqrs;
    std::vector<Move> checkEscapes;
    std::unordered_map<Player, std::vector<GridVector>> doublePushedPawn;

public:

    Board();
    void setup();

    MoveCallback move(Move pieceMove);
    
    Piece getSqrPiece(GridVector sqr);
    Player getSqrOwner(GridVector sqr);
    bool emptySqr(GridVector sqr);
    bool validSqr(GridVector sqr);

    Player getCheck();
    Player getPlayerToMove();
    Status getStatus();
    Player getWinner();
    

private:

    void step();

    int ind(GridVector sqr);
    void setSqr(GridVector sqr, Piece type, Player owner);
    void clearSqr(GridVector sqr);
    void execute(Move pieceMove);

    void updateSqrCoverage();
    std::vector<GridVector> getRay(GridVector origin, GridVector dir);
    void updateCheckEscapes();
    MoveCallback validateMove(Move pieceMove);

};

// [PUBLIC] ctor for board
Board::Board()
{

    // INTIALISE PIECE SETTINGS
    // Pawn
    moveTrajs[PAWN] = {}; // pawn move options are defined in function
    moveConstraint[PAWN] = true;

    // Rook
    moveTrajs[ROOK] = {
        {0,1},
        {0,-1},
        {1,0},
        {-1,0},
    };
    moveConstraint[ROOK] = false;

    // Knight
    moveTrajs[KNIGHT] = {
        {-2,-1},
        {-1,-2},
        {2,-1},
        {-1,2},
        {-2,1},
        {1,-2},
        {2,1},
        {1,2},
    };
    moveConstraint[KNIGHT] = true;

    // Bishop
    moveTrajs[BISHOP] = {
        {-1,-1},
        {1,-1},
        {-1,1},
        {1,1},
    };
    moveConstraint[BISHOP] = false;

    // Queen
    moveTrajs[QUEEN] = {
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
    moveTrajs[KING] = {
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

// [PUBLIC]
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

    kingSqr[WHITE] = GridVector({4,0});
    kingSqr[BLACK] = GridVector({4,7});

    playerToMove = WHITE;
    status = IN_PROGRESS;
    check = PLAYER_NULL;
    winner = PLAYER_NULL;

    step(); // initially step system
}

/* ======================================== Basic operations ======================================== */

// [PRIVATE]
int Board::ind(GridVector sqr)
{
    return sqr.rank*8 + sqr.file;
}

// [PUBLIC]
Piece Board::getSqrPiece(GridVector sqr)
{
    return sqrPieces[ind(sqr)];
}

// [PUBLIC]
Player Board::getSqrOwner(GridVector sqr)
{
    return sqrOwners[ind(sqr)];
}

// [PUBLIC]
bool Board::emptySqr(GridVector sqr)
{
    return (sqrPieces[ind(sqr)] == PIECE_NULL);
}

// [PUBLIC]
bool Board::validSqr(GridVector sqr)
{
    return ((sqr.file >= 0) && (sqr.file < 8) && (sqr.rank >= 0) && (sqr.rank < 8));
}

// [PRIVATE]
void Board::setSqr(GridVector sqr, Piece type, Player owner)
{
    sqrPieces[ind(sqr)] = type;
    sqrOwners[ind(sqr)] = owner;
}

// [PRIVATE]
void Board::clearSqr(GridVector sqr)
{
    sqrPieces[ind(sqr)] = PIECE_NULL;
    sqrOwners[ind(sqr)] = PLAYER_NULL;
}

// [PRIVATE]
void Board::execute(Move pieceMove)
{
    Piece piece = sqrPieces[ind(pieceMove.start)];
    Player owner = sqrOwners[ind(pieceMove.start)];
    clearSqr(pieceMove.start);
    sqrPieces[ind(pieceMove.end)] = piece;
    sqrOwners[ind(pieceMove.end)] = owner;

    // keep track of the king positions
    if (pieceMove.start == kingSqr[playerToMove])
    {
        kingSqr[playerToMove] = pieceMove.end;
    }
}

// [PUBLIC] returns which player in check, if any
Player Board::getCheck() 
{
    return check;
}

// [PUBLIC] returns player to move
Player Board::getPlayerToMove()
{
    return playerToMove;
}

// [PUBLIC] returns game status
Status Board::getStatus()
{
    return status;
}

// [PUBLIC] returns game winner
Player Board::getWinner()
{
    return winner;
}

/* ======================================== Game mechanics ======================================== */

// [PUBLIC] primary function for moving pieces from an outside consol
MoveCallback Board::move(Move pieceMove)
{
    MoveCallback cb = validateMove(pieceMove);
    if (cb == SUCCESS)
    {
        execute(pieceMove);
        step();
        playerToMove = !playerToMove;
    }
    return cb;
}

// [PRIVATE] step the board system to update status for both players' pieces
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

    // update the square coverage after a move to assess current position on board
    updateSqrCoverage();        

    // if a player is in check, generate the possible check escapes
    if (check != PLAYER_NULL)
    {
        updateCheckEscapes();

        if (checkEscapes.size() == 0)
        {
            status = CHECKMATE;
            winner = !check;
        }
    }

    /*
    std::cout << "Player in check: " << check << std::endl;
    for (auto& sqr : pinnedSqrs)
        std::cout << "Pinned sqr: " << sqr << std::endl;
    for (auto& sqr : checkRaySqrs)
        std::cout << "Check ray sqr: " << sqr << std::endl;
    for (auto& mv : checkEscapes)
        std::cout << "Check escape move: " << mv << std::endl;
    */
}

// [PRIVATE] returns ray generated by a move direction from a piece origin
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

// [PRIVATE] updates the coverage on each square by each players' pieces
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
                        for (auto& mv : moveTrajs[piece])
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
                    for (auto& mv : moveTrajs[piece])
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
                            //else
                            //    std::cout << "CATASTROPHIC ERROR - BOTH KINGS HAVE BECOME CHECKED!" << std::endl;


                            // include check ray squares
                            for (int k = 0; k < enemyKingInRay; ++k)
                            {
                                checkRaySqrs.push_back(ray[k]);
                            }
                        }
                        else if ((enemyKingInRay > -1) && (friendsInRay.size() == 0) && (enemiesInRay.size() == 1) && (enemiesInRay[0] < enemyKingInRay))
                        {
                            // pin this piece
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

// [PRIVATE] get check escapes (assuming a player is in check)
void Board::updateCheckEscapes()
{
    // get number of pieces checking the king
    std::vector<GridVector> checkers = {};
    for (auto& cv : sqrCoverage[ind(kingSqr[check])]) // iterate through covers on the checked king's square
    {
        if (cv.owner == !check)
        {
            checkers.push_back(cv.origin);
        }
    }
    
    // look at move options for the king
    for (auto& mv : moveTrajs[KING]) // iterate through king move options
    {
        GridVector sqr = kingSqr[check] + mv;
        if ((validSqr(sqr)) && (getSqrOwner(sqr) != check)) // if the square is valid and is not occupied by checked player (free or enemy piece)
        {
            bool sqrFree = true;
            for (auto& cv : sqrCoverage[ind(sqr)]) // check the square is not covered by enemy piece
            {
                if (cv.owner == !check)
                {
                    sqrFree = false;
                }
            }
            if (sqrFree == true)
            {
                checkEscapes.push_back({kingSqr[check], sqr});
            }
        }
    }

    if (checkers.size() == 1)
    {
        // look at taking the checking piece
        for (auto& cv : sqrCoverage[ind(checkers[0])]) // iterate through covers on the checking piece's square
        {
            if (cv.owner == check) // if owned by checked player (includes the king)
            {
                checkEscapes.push_back({cv.origin, checkers[0]});
            }
        }

        // look at blocking the checking piece
        // check this piece isn't pinned
        for (auto& sqr : checkRaySqrs) // iterate through ray squares
        {
            for (auto& cv : sqrCoverage[ind(sqr)]) // iterate through covers on square
            {
                if ((cv.owner == check) && (cv.piece != KING)) // check if cover belongs to checked player and that this isn't the king
                {
                     // check piece isn't pinned
                    bool isPinned = false;
                    for (auto& psqr : pinnedSqrs)
                    {
                        if (sqr == psqr)
                            isPinned = true;
                    }
                    if (isPinned == false)
                        checkEscapes.push_back({cv.origin, sqr});
                }
            }
        }
    }
}  

// [PRIVATE] validates move by checking if the piece at move.start covers move.end
// ensures that the piece in question isn't pinned
// if the player is in check, this move must move them out of check
MoveCallback Board::validateMove(Move pieceMove)
{
    // if game over, then return false
    if (status != IN_PROGRESS) 
        return GAME_OVER;

    // ensure that both squares provided in move are valid squares on the board and are indexed properly
    if ( (!(validSqr(pieceMove.start))) || (!(validSqr(pieceMove.end))) )
        return INVALID_SQR;

    // ensure that the piece being moved belongs to the player to move
    if (sqrOwners[ind(pieceMove.start)] != playerToMove)
        return ENEMY_PIECE;

    // check if owner is in check and whether this move is an available move
    if (check == playerToMove)
    {
        bool escapesCheck = false;
        for (auto& mv : checkEscapes)
        {
            if (pieceMove == mv)
                escapesCheck = true;
        }
        if (escapesCheck == false)
            return CHECK_NOT_ESCAPED;
    }
    else // if player not in check
    {
        // check that this piece covers this square
        bool covers = false;
        for (auto& cvr : sqrCoverage[ind(pieceMove.end)])
        {
            if (cvr.origin == pieceMove.start)
            {
                covers = true;
            }
        }
        if (covers == false)
            return ILLEGAL_TRAJ;

        // check if piece is pinned
        bool isPinned = false;
        for (auto& sqr : pinnedSqrs)
        {
            if (pieceMove.start == sqr)
                isPinned = true;
        }
        if (isPinned == true)
            return PINNED_PIECE;
    }
    
    return SUCCESS;
}
