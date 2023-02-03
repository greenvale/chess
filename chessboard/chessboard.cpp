#include "chessboard.h"

namespace chessboard
{

/**************************************************************************************/
// GRIDVECTOR STRUCT

GridVector operator+(GridVector lh, GridVector rh)
{
    return GridVector(lh.file + rh.file, lh.rank + rh.rank);
}
GridVector operator-(GridVector lh, GridVector rh)
{
    return GridVector(lh.file - rh.file, lh.rank - rh.rank);
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
bool operator!=(GridVector lh, GridVector rh)
{
    return !(lh == rh);
}

/**************************************************************************************/
// PIECE ENUM

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

std::ostream& operator<<(std::ostream& os, Status sts)
{
    switch(sts)
    {
        case IN_PROGRESS: os << "In progress"; break;
        case CHECKMATE: os << "Checkmate"; break;
        case DRAW: os << "Draw"; break;
        case STALEMATE: os << "Stalemate"; break;
    }
    return os;
}

/**************************************************************************************/
// MOVE STRUCT

std::ostream& operator<<(std::ostream& os, Move mv)
{
    os << "(" << mv.start.file << ", " << mv.start.rank << ") -> (" << mv.end.file << ", " << mv.end.rank << ")";
    return os;
}
bool operator==(Move lh, Move rh)
{
    return ((lh.start.file == rh.start.file) && (lh.start.rank == rh.start.rank) && (lh.end.file == rh.end.file) && (lh.end.rank == rh.end.rank));
}
bool operator!=(Move lh, Move rh)
{
    return !(lh == rh);
}

/**************************************************************************************/
// MOVE CALLBACK ENUM

std::ostream& operator<<(std::ostream& os, MoveCallback t)
{
    switch(t)
    {
        case SUCCESS: os << "Success"; break;
        case FAILURE: os << "Failure"; break;

    }
    return os; 
}

/**************************************************************************************/
// BOARD

// [PUBLIC] ctor for board
Board::Board()
{

    // INTIALISE PIECE SETTINGS
    // Pawn
    moveVectors[PAWN] = {}; // pawn move options are defined in function
    moveConstraint[PAWN] = true;

    // Rook
    moveVectors[ROOK] = { {0,1}, {0,-1}, {1,0}, {-1,0} };
    moveConstraint[ROOK] = false;

    // Knight
    moveVectors[KNIGHT] = { {-2,-1}, {-1,-2}, {2,-1}, {-1,2}, {-2,1}, {1,-2}, {2,1}, {1,2} };
    moveConstraint[KNIGHT] = true;

    // Bishop
    moveVectors[BISHOP] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };
    moveConstraint[BISHOP] = false;

    // Queen
    moveVectors[QUEEN] = { {0,1}, {0,-1}, {1,0}, {-1,0}, {-1,-1}, {1,-1}, {-1,1}, {1,1} };
    moveConstraint[QUEEN] = false;

    // King
    moveVectors[KING] = { {0,1}, {0,-1}, {1,0}, {-1,0}, {-1,-1}, {1,-1}, {-1,1}, {1,1} };
    moveConstraint[KING] = true;

    // Initialise empty board
    sqrPieces = std::vector<Piece>(64, PIECE_NULL);
    sqrOwners = std::vector<Player>(64, PLAYER_NULL);

    // functionality assets
    sqrCoverage = std::vector<std::vector<SqrCover>>(64, std::vector<SqrCover>());
    validMoves = std::vector<std::vector<Move>>(64, std::vector<Move>());
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
    kingMoved[WHITE] = false;
    kingMoved[BLACK] = false;
    rookKSMoved[WHITE] = false;
    rookKSMoved[BLACK] = false;
    rookQSMoved[WHITE] = false;
    rookQSMoved[BLACK] = false;

    plrToMove = WHITE;
    status = IN_PROGRESS;
    check = PLAYER_NULL;
    winner = PLAYER_NULL;

    evaluateBoard(); // initially step system
}

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
void Board::executeMove(Move mv)
{
    Piece piece = sqrPieces[ind(mv.start)];
    Player owner = sqrOwners[ind(mv.start)];
    clearSqr(mv.start);
    setSqr(mv.end, piece, owner);

    // keep track of the king positions
    if (mv.start == kingSqr[plrToMove])
    {
        kingSqr[plrToMove] = mv.end;
    }
}

// [PRIVATE] executes en passant move
void Board::executeEnPssnt(Move mv)
{
    if ((mv.end - mv.start).rank == 1) // white taking black pawn
    {
        clearSqr(mv.end - GridVector(0, 1));
        executeMove(mv);
    }
}

// [PRIVATE]
void Board::executeCastleKS()
{
    int rank = (plrToMove == WHITE) ? 0 : 7;
    clearSqr({4,rank});
    clearSqr({7,rank});
    setSqr({6,rank}, KING, plrToMove);
    setSqr({5,rank}, ROOK, plrToMove);
    kingSqr[plrToMove] = GridVector({6,rank}); // update kjng pos
}

// [PRIVATE] 
void Board::executeCastleQS()
{
    int rank = (plrToMove == WHITE) ? 0 : 7;
    clearSqr({4,rank});
    clearSqr({0,rank});
    setSqr({2,rank}, KING, plrToMove);
    setSqr({3,rank}, ROOK, plrToMove);
    kingSqr[plrToMove] = GridVector({2,rank}); // update king pos
}

// [PUBLIC] returns which player in check, if any
Player Board::getCheck() 
{
    return check;
}

// [PUBLIC] returns player to move
Player Board::getPlayerToMove()
{
    return plrToMove;
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

// [PUBLIC]m
std::vector<Move> Board::getValidMoves(GridVector sqr)
{
    return validMoves[ind(sqr)];
}

// [PUBLIC] primary function for moving pieces from an outside program
MoveCallback Board::requestMove(Move mv, Piece pieceFlag)
{
    MoveCallback cb = FAILURE;

    if (status == IN_PROGRESS)
    {
        bool skip = false;

        // check if en passant move
        if (skip == false)
        {
            for (auto& epMv : enpssntMoves)
            {
                if (mv == epMv && getSqrOwner(epMv.start) == plrToMove) // check that this move can be made by player to move
                {
                    executeEnPssnt(mv);
                    skip = true;
                    cb = SUCCESS;
                }
            }
        }

        // check if castling move
        if (skip == false)
        {
            if (kingSqr[plrToMove] == mv.start && castleKSValid == true && (mv.end - mv.start).file == 2)
            {
                executeCastleKS();
                skip = true;
                cb = SUCCESS;
            }
            else if (kingSqr[plrToMove] == mv.start && castleQSValid == true && (mv.end - mv.start).file == -2)
            {
                executeCastleQS();
                skip = true;
                cb = SUCCESS;
            }
        }

        // if not a special move (en passant/castle) then treat as a normal move and search through valid moves
        if (skip == false)
        {
            for (auto& validMv : validMoves[ind(mv.start)])
            {
                if (mv == validMv)
                {
                    cb = SUCCESS;
                }
            }
        }

        if (cb == SUCCESS) // move has been validated
        {
            updateEnPssnt(mv); // update en passant before executing move
            
            // keep track of king being moved for first time for castling
            if (mv.start == kingSqr[plrToMove]) // if plr moves king then keep track of this
            {
                kingMoved[plrToMove] = true;
            }

            // keep track of rook being moved for first time for castling
            int rank = (plrToMove == WHITE) ? 0 : 7;
            if (mv.start == GridVector(7,rank) && rookKSMoved[plrToMove] == false)
            {
                rookKSMoved[plrToMove] = true;
            }
            else if (mv.start == GridVector(0,rank) && rookQSMoved[plrToMove] == false)
            {
                rookQSMoved[plrToMove] = true;
            }

            // execute move
            if (skip == false) // if move already executed in a special way then skip = true
            {
                executeMove(mv);
            }

            // check if this move is a pawn promotion and thus requires pieceFlag
            if (getSqrPiece(mv.end) == PAWN && mv.end.rank == (7 - rank))
            {
                setSqr(mv.end, pieceFlag, plrToMove);
            }
            
            // switch player to move and reevaluate board
            plrToMove = !plrToMove;
            evaluateBoard(); // evaluates board (en passant moves already calculated)
        }
    }
    return cb;
}

// [PRIVATE] returns ray generated by a move vector from a piece origin
std::vector<GridVector> Board::castRay(GridVector origin, GridVector vec)
{
    std::vector<GridVector> ray = {};
    bool stop = false;
    int step = 1;
    while (stop == false)
    {
        if (validSqr(origin + vec*step) == true)
        {
            ray.push_back(origin + vec*step);
            step++;
        }
        else
        {
            stop = true;
        }
    }
    return ray;
}

// [PRIVATE] returns true if a square is covered by a player
bool Board::isCoveredByPlr(GridVector sqr, Player plr)
{
    for (auto& cvr : sqrCoverage[ind(sqr)])
    {
        if (cvr.owner == plr)
            return true;
    }
    return false;
}

// [PRIVATE] returns true if a square is covered by a player
bool Board::isCaptureCoveredByPlr(GridVector sqr, Player plr, bool allowRayBndKing)
{
    for (auto& cvr : sqrCoverage[ind(sqr)])
    {
        if (cvr.owner == plr && (cvr.type == CAPTURE || cvr.type == PUSH_CAPTURE || (allowRayBndKing == true && cvr.type == RAY_BEYOND_KING)))
            return true;
    }
    return false;
}

// [PRIVATE] returns player's pieces covering this square
std::vector<SqrCover> Board::getCoversByPlr(GridVector sqr, Player plr)
{
    std::vector<SqrCover> cvrs = {};
    for (auto& cvr : sqrCoverage[ind(sqr)])
    {
        if (cvr.owner == plr)
            cvrs.push_back(cvr);
    }
    return cvrs;
}

// [PRIVATE] returns true if a square is covered by another square
bool Board::isCoveredBySqr(GridVector on, GridVector by)
{
    for (auto& cvr : sqrCoverage[ind(on)])
    {
        if (cvr.origin == by && (cvr.type == CAPTURE || cvr.type == PUSH_CAPTURE || cvr.type == RAY_BEYOND_KING))
            return true;
    }
    return false;
}

// [PRIVATE] returns true if piece on given sqr is pinned
bool Board::isPinned(GridVector sqr)
{
    for (auto& pin : pinRays)
    {
        if (pin.on == sqr)
            return true;
    }
    return false;
}

// [PUBLIC] returns number of valid moves for the player to move
int Board::getNumValidMoves()
{
    int n = 0;
    for (auto& mvs : validMoves)
    {
        n += mvs.size();
    }
    return n;
}

// [PRIVATE] steps the system by clearing all previous data and recalculating position to get coverage of pieces, king threats, etc.
void Board::evaluateBoard()
{
    // clear data
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            sqrCoverage[ind({i,j})].clear(); // refresh the coverage vectors for each square to update square coverage
            validMoves[ind({i,j})].clear();
        }
    }
    check = PLAYER_NULL;
    pinRays.clear();    
    checkRays.clear();

    // update the square coverage after a move to assess current position on board
    updateSqrCoverage();

    // determine if player to move is in check
    if (isCoveredByPlr(kingSqr[plrToMove], !plrToMove))
    {
        check = plrToMove;
    }

    // update king rays for player to move (pinned rays, check rays)
    updateKingRays(ROOK); // file/rank rays
    updateKingRays(BISHOP); // diagonal rays
    
    updateCastle(); // update castle moves before updating valid moves (as this will be included)

    updateValidMoves(); // update valid moves for player to move

    // if no valid moves then game is over
    if (getNumValidMoves() == 0)
    {
        if (check == plrToMove)
        {
            // check mate
            status = CHECKMATE;
            winner = !plrToMove;
        }
        else
        {
            // stale mate
            status = STALEMATE;
        }
    }
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
                            sqrCoverage[ind(GridVector(i,j + sign))].push_back({ {i,j}, piece, owner, PUSH });

                            // double pawn push
                            if ((((owner == WHITE) && (j == 1)) || ((owner == BLACK) && (j == 6))) && (validSqr(GridVector(i,j + 2*sign))) && (getSqrOwner(GridVector(i,j + 2*sign)) == PLAYER_NULL))
                            {
                                sqrCoverage[ind(GridVector(i,j + 2*sign))].push_back({ {i,j}, piece, owner, PUSH });
                            }
                        }

                        // pawn capture
                        if (validSqr(GridVector(i - 1,j + sign)))
                        {
                            sqrCoverage[ind(GridVector(i - 1,j + sign))].push_back({ {i,j}, piece, owner, CAPTURE });
                        }
                        if (validSqr(GridVector(i + 1,j + sign)))
                        {
                            sqrCoverage[ind(GridVector(i + 1,j + sign))].push_back({ {i,j}, piece, owner, CAPTURE });
                        }
                    }
                    // KNIGHT / KING
                    else
                    {
                        for (auto& vec : moveVectors[piece])
                        {
                            if (validSqr(GridVector(i,j) + vec))
                            {
                                sqrCoverage[ind(GridVector(i,j) + vec)].push_back({ {i,j}, piece, owner, PUSH_CAPTURE }); // the piece covers this square, add to coverage
                            }
                        }
                    }
                }
                else // MOVE UNCONSTRAINED PIECE (ROOK, BISHOP, QUEEN)
                {
                    for (auto& vec : moveVectors[piece])
                    {
                        std::vector<GridVector> ray = castRay({i,j}, vec); // returns a vector of squares from origin in direction of move trajectory

                        bool enemyKingInRay = false;
                        for (auto& sqr : ray)
                        {
                            if (enemyKingInRay == false)
                                sqrCoverage[ind(sqr)].push_back({ {i,j}, piece, owner, PUSH_CAPTURE });
                            else
                                sqrCoverage[ind(sqr)].push_back({ {i,j}, piece, owner, RAY_BEYOND_KING });
                            
                            if (getSqrOwner(sqr) != PLAYER_NULL)
                            {
                                if (sqr == kingSqr[!getSqrOwner({i,j})])
                                {
                                    enemyKingInRay = true; // if enemy king in ray then keep going to find unsafe squares behind king
                                }
                                else
                                {
                                    break; // stop loop if enemy/friendly piece enounctered on sqr in ray
                                }
                            }
                            else if (enemyKingInRay == true)
                            {
                                break; // stop loop after going 1 square beyond enemy king
                            }
                        }
                    }
                }
            }
        }
    }
}

// [PRIVATE] updates the pin rays and check rays coming from the king of a given player given a "dirPiece"
// dirPiece must be either Rook (files/ranks) or Bishop (diagonals) and is used to cast rays from the king's position to identify enemy unconstrained pieces
// these pieces could be checking the king or pinning pieces in front of the king
// check rays need to be recorded to find the valid moves out of check that don't land the king back in check
void Board::updateKingRays(Piece dirPiece)
{
    for (auto& vec : moveVectors[dirPiece]) // iterate through diagonal vectors
    {
        std::vector<GridVector> ray = castRay(kingSqr[plrToMove], vec);

        bool stop = false;
        int friendInRay = -1;
        
        for (int k = 0; k < ray.size(); ++k) // loop through squares in ray
        {
            if (getSqrOwner(ray[k]) == plrToMove)
            {
                // friendly piece in ray, if first piece encountered then record, else stop traversal
                if (friendInRay > -1)
                    stop = true;
                else
                    friendInRay = k;
            }
            else if (getSqrOwner(ray[k]) == !plrToMove && (getSqrPiece(ray[k]) == dirPiece || getSqrPiece(ray[k]) == QUEEN))
            {
                // get ray squares (squares between king and enemy piece)
                std::vector<GridVector> raySqrs = {};
                for (int l = 0; l <= k; ++l)
                {
                    raySqrs.push_back(ray[l]);
                }

                if (friendInRay > -1)
                {
                    // if friend in ray then the friend is pinned
                    pinRays.push_back({ray[friendInRay], ray[k], raySqrs});
                }
                else
                {
                    // if no friend in ray then this is a check ray
                    checkRays.push_back({kingSqr[plrToMove], ray[k], raySqrs});
                }

                stop = true;
            }
            else if (getSqrOwner(ray[k]) == !plrToMove)
            {
                // non-threatening enemy piece in ray
                stop = true;
            }

            if (stop == true)
                break;
        }
    }
}

// [PRIVATE]
void Board::updateValidMoves()
{
    if (check == plrToMove)
    {
        // IN CHECK
        // find ways to get out of check
        std::vector<SqrCover> checkerCvrs = getCoversByPlr(kingSqr[plrToMove], !plrToMove); // get pieces that are checking the king
        
        // method 1 : move the king to a square that is not defended by enemy (including taking piece checking (if next to) or other enemy piece that isn't defended)
        for (auto& vec : moveVectors[KING])
        {
            if (validSqr(kingSqr[plrToMove] + vec) && getSqrOwner(kingSqr[plrToMove] + vec) != plrToMove && (isCaptureCoveredByPlr(kingSqr[plrToMove] + vec, !plrToMove, true) == false))
            {
                validMoves[ind(kingSqr[plrToMove])].push_back(Move(kingSqr[plrToMove], kingSqr[plrToMove] + vec));
            }
        }
        if (checkerCvrs.size() == 1)
        {
            // method 2 : take the checking piece with something other than the king (if only 1 piece checking)
            std::vector<SqrCover> cvrsOnChecker = getCoversByPlr(checkerCvrs[0].origin, plrToMove);
            for (auto& cvr : cvrsOnChecker)
            {
                if (isPinned(cvr.origin) == false && (cvr.type == CAPTURE || cvr.type == PUSH_CAPTURE) && (cvr.piece != KING)) // a pinned piece cannot be pinned by the single checker of the king, can't be PUSH or RAY_BEYOND_KING
                {
                    validMoves[ind(cvr.origin)].push_back(Move(cvr.origin, checkerCvrs[0].origin));
                }
            }

            // method 3 : blocking the check if it originates from unconstrained piece (if only 1 piece checking)
            // assume the size of the checkRay vector is at most 1 (otherwise there would be multiple checkers)
            if (checkRays.size() == 1)
            {
                for (auto& sqr : checkRays[0].raySqrs)
                {
                    std::vector<SqrCover> blocksOnSqr = getCoversByPlr(sqr, plrToMove);
                    for (auto& cvr : blocksOnSqr)
                    {
                        if (cvr.origin != kingSqr[plrToMove] && (cvr.type == PUSH || cvr.type == PUSH_CAPTURE)) // cannot be blocked by king itself
                        {
                            validMoves[ind(cvr.origin)].push_back(Move(cvr.origin, sqr));
                        }
                    }
                }
            }
        }
    }   
    else
    {
        // NOT IN CHECK, therefore iterate through all squares and record moves that cover this square by the player to move
        // only calculate legal moves for non-king pieces that are NOT PINNED
        for (int i = 0; i < 8; ++i)
        {
            for (int j = 0; j < 8; ++j)
            {
                Piece piece = getSqrPiece({i,j});
                Player owner = getSqrOwner({i,j});

                if (owner != plrToMove) // make sure this square is not owned by plr to move (player to move cannot move piece to square it already occupies)
                {
                    for (auto& cvr : sqrCoverage[ind({i,j})])
                    {
                        if (cvr.owner == plrToMove && isPinned(cvr.origin) == false) // make sure this cover is owned by plr to move and is NOT PINNED
                        {
                            if (cvr.piece == PAWN)
                            {
                                if (owner == PLAYER_NULL && cvr.type == PUSH)
                                {
                                    // if pawn then push move only allowed if square empty
                                    validMoves[ind(cvr.origin)].push_back(Move(cvr.origin, {i,j}));
                                }
                                else if (owner == !plrToMove && cvr.type == CAPTURE)
                                {
                                    // capture move only allowed if square occupied
                                    validMoves[ind(cvr.origin)].push_back(Move(cvr.origin, {i,j}));
                                }
                            }
                            else if (cvr.piece != KING)
                            {
                                // for all non-pawn pieces, the capture and push are coincidental
                                validMoves[ind(cvr.origin)].push_back(Move(cvr.origin, {i,j}));
                            }
                        }
                    }
                }
            }
        }

        // calculate legal moves for player to move's king directly as it's more efficient than looking at cvrs by the king
        for (auto& vec : moveVectors[KING])
        {
            if (validSqr(kingSqr[plrToMove] + vec))
            {
                if ((isCaptureCoveredByPlr(kingSqr[plrToMove] + vec, !plrToMove, false) == false) && getSqrOwner(kingSqr[plrToMove] + vec) != plrToMove)
                {
                    validMoves[ind(kingSqr[plrToMove])].push_back(Move(kingSqr[plrToMove], kingSqr[plrToMove] + vec));
                }
            }
        }

        // calculate legal moves for pinned pieces, they can only move if they are moving to another square on the pin ray (including taking the pinning piece)
        // note this code could be made more efficient by looking at piece types and going case by case
        for (auto& pin : pinRays)
        {
            // iterate through each square in ray
            for (auto& sqr : pin.raySqrs)
            {
                // iterate through each cover if square
                for (auto& cvr : sqrCoverage[ind(sqr)])
                {
                    if (cvr.origin == pin.on) // check this cover is owned by piece that is pinned
                    {
                        if (getSqrPiece(pin.on) == PAWN)
                        {
                            // if a pawn then must distinguish between capture and push
                            if (getSqrOwner(sqr) == PLAYER_NULL && cvr.type == PUSH)
                            {
                                // if pawn then push move only allowed if square empty
                                validMoves[ind(pin.on)].push_back(Move(pin.on, sqr));
                            }
                            else if (getSqrOwner(sqr) == !plrToMove && cvr.type == CAPTURE)
                            {
                                // capture move only allowed if square occupied
                                validMoves[ind(pin.on)].push_back(Move(pin.on, sqr));
                            }
                        }
                        else
                        {
                            validMoves[ind(pin.on)].push_back(Move(pin.on, sqr));
                        }
                    }
                }
            }
        }

        // include en passant moves that are already calculated for the player to move
        for (auto& mv : enpssntMoves)
        {
            validMoves[ind(mv.start)].push_back(mv);
        }

        // include castling moves using the flags that have already been calculated
        if (castleKSValid == true)
        {
            validMoves[ind(kingSqr[plrToMove])].push_back(Move(kingSqr[plrToMove], kingSqr[plrToMove] + GridVector(2,0)));
        }
        if (castleQSValid == true)
        {
            validMoves[ind(kingSqr[plrToMove])].push_back(Move(kingSqr[plrToMove], kingSqr[plrToMove] + GridVector(-2,0)));
        }
    }
}

// called just after move approved for execution but before it is executed and the plrToMove is changed
void Board::updateEnPssnt(Move mv)
{
    enpssntMoves.clear(); // note en passant must be exercised immediately, clear all previous moves

    // check if there are new enpassant moves available with latest move
    if (getSqrPiece(mv.start) == PAWN)
    {
        if (plrToMove == WHITE && mv.start.rank == 1 && (mv.end - mv.start) == GridVector(0,2)) // if white double pawn push
        {
            if (validSqr(mv.start + GridVector(1,2)) && getSqrPiece(mv.start + GridVector(1,2)) == PAWN && getSqrOwner(mv.start + GridVector(1,2)) == !plrToMove)
            {
                enpssntMoves.push_back(Move(mv.start + GridVector(1,2), mv.start + GridVector(0,1)));
            }
            else if (validSqr(mv.start + GridVector(-1,2)) && getSqrPiece(mv.start + GridVector(-1,2)) == PAWN && getSqrOwner(mv.start + GridVector(-1,2)) == !plrToMove)
            {
                enpssntMoves.push_back(Move(mv.start + GridVector(-1,2), mv.start + GridVector(0,1)));
            }
        }
        else if (plrToMove == BLACK && mv.start.rank == 6 && (mv.end - mv.start) == GridVector(0,-2)) // if black double pawn push
        {
            if (validSqr(mv.start + GridVector(1,-2)) && getSqrPiece(mv.start + GridVector(1,-2)) == PAWN && getSqrOwner(mv.start + GridVector(1,-2)) == !plrToMove)
            {
                enpssntMoves.push_back(Move(mv.start + GridVector(1,-2), mv.start + GridVector(0,-1)));
            }
            else if (validSqr(mv.start + GridVector(-1,-2)) && getSqrPiece(mv.start + GridVector(-1,-2)) == PAWN && getSqrOwner(mv.start + GridVector(-1,-2)) == !plrToMove)
            {
                enpssntMoves.push_back(Move(mv.start + GridVector(-1,-2), mv.start + GridVector(0,-1)));
            }
        }
    }
}

void Board::updateCastle()
{
    // reset flags to false
    castleKSValid = false;
    castleQSValid = false;

    int rank = (plrToMove == WHITE) ? 0 : 7;

    if (kingMoved[plrToMove] == false)
    {
        // king side castling
        if (rookKSMoved[plrToMove] == false && getSqrPiece({5,rank}) == PIECE_NULL && getSqrPiece({6,rank}) == PIECE_NULL 
            && isCoveredByPlr({5,rank}, !plrToMove) == false && isCoveredByPlr({6,rank}, !plrToMove) == false)
        {
            castleKSValid = true;
        }
        // queen side castling
        if (rookKSMoved[plrToMove] == false && getSqrPiece({1,rank}) == PIECE_NULL && getSqrPiece({2,rank}) == PIECE_NULL && getSqrPiece({3,rank}) == PIECE_NULL
            && isCoveredByPlr({1,rank}, !plrToMove) == false && isCoveredByPlr({2,rank}, !plrToMove) == false && isCoveredByPlr({3,rank}, !plrToMove) == false)
        {
            castleQSValid = true;
        }
    }
}

}