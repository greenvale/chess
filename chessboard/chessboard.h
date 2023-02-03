/* Chess library */
#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cstring>
#include <algorithm>

namespace chessboard
{

struct GridVector
{
    int file, rank;

    GridVector() : file (999), rank(999) {}
    GridVector(const int& file, const int& rank) : file(file), rank(rank) {}

};

GridVector operator+(GridVector lh, GridVector rh);
GridVector operator*(GridVector lh, double rh);
GridVector operator*(double lh, GridVector rh);
std::ostream& operator<<(std::ostream& os, GridVector gv);
bool operator==(GridVector lh, GridVector rh);
bool operator!=(GridVector lh, GridVector rh);

enum Piece {
    PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING, PIECE_NULL
};

std::ostream& operator<<(std::ostream& os, Piece t);

enum Player {
    WHITE, BLACK, PLAYER_NULL
};

std::ostream& operator<<(std::ostream& os, Player t);
Player operator!(Player p);

enum Status
{
    IN_PROGRESS, CHECKMATE, DRAW, STALEMATE
};

std::ostream& operator<<(std::ostream& os, Status sts);

struct Move
{
    GridVector start; 
    GridVector end;

    Move() {}
    Move(GridVector start, GridVector end) : start(start), end(end) {}
};

std::ostream& operator<<(std::ostream& os, Move mv);
bool operator==(Move lh, Move rh);
bool operator!=(Move lh, Move rh);

enum CoverType
{
    PUSH, CAPTURE, PUSH_CAPTURE, RAY_BEYOND_KING
};

struct SqrCover
{
    GridVector origin;
    Piece piece;
    Player owner;
    CoverType type;

    SqrCover() {}
    SqrCover(GridVector origin, Piece piece, Player owner, CoverType type) : origin(origin), piece(piece), owner(owner), type(type) {}
};

struct Ray
{
    GridVector on;
    GridVector by;
    std::vector<GridVector> raySqrs;

    Ray() {}
    Ray(GridVector on, GridVector by, std::vector<GridVector> raySqrs) : on(on), by(by), raySqrs(raySqrs) {}
};

enum MoveCallback
{
    SUCCESS, FAILURE
};

std::ostream& operator<<(std::ostream& os, MoveCallback t);

class Board
{

private:
    std::vector<Piece> sqrPieces;
    std::vector<Player> sqrOwners;

    // Static rules/assets
    std::unordered_map<Piece, std::vector<GridVector>> moveVectors;
    std::unordered_map<Piece, bool> moveConstraint;

    // Flags
    Player plrToMove;
    Player winner;
    Status status;
    Player check;
    std::unordered_map<Player, bool> kingMoved;
    std::unordered_map<Player, bool> rookKSMoved;
    std::unordered_map<Player, bool> rookQSMoved;
    std::unordered_map<Player, GridVector> kingSqr;

    // general square coverage (doesn't equate to legal moves!)
    std::vector<std::vector<SqrCover>> sqrCoverage;
    
    // rays
    std::vector<Ray> pinRays;
    std::vector<Ray> checkRays;

    std::vector<std::vector<Move>> validMoves; // stores all valid moves including special moves

    // special moves
    std::vector<Move> enpssntMoves;
    bool castleKSValid;
    bool castleQSValid;

public:
    Board();
    void setup();

    Piece getSqrPiece(GridVector sqr);
    Player getSqrOwner(GridVector sqr);
    bool emptySqr(GridVector sqr);
    bool validSqr(GridVector sqr);

    Player getCheck();
    Player getPlayerToMove();
    Status getStatus();
    Player getWinner();
    std::vector<Move> getValidMoves(GridVector sqr);
    int getNumValidMoves();
    
    MoveCallback requestMove(Move mv);
    
private:
    void evaluateBoard();

    int ind(GridVector sqr);
    void setSqr(GridVector sqr, Piece type, Player owner);
    void clearSqr(GridVector sqr);
    void executeMove(Move mv);
    void executeEnPssnt(Move mv);
    void executeCastleKS();
    void executeCastleQS();

    bool isCoveredByPlr(GridVector sqr, Player plr);
    bool isCaptureCoveredByPlr(GridVector sqr, Player plr, bool allowRayBndKing);
    bool isCoveredBySqr(GridVector on, GridVector by);
    bool isPinned(GridVector sqr);
    std::vector<SqrCover> getCoversByPlr(GridVector sqr, Player plr);

    std::vector<GridVector> castRay(GridVector origin, GridVector dir);

    void updateSqrCoverage();
    void updateKingRays(Piece dirPiece);
    void updateValidMoves();
    void updateEnPssnt(Move mv);
    void updateCastle();

};

}