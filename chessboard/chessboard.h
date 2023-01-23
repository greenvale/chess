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

GridVector operator+(GridVector lh, GridVector rh);
GridVector operator*(GridVector lh, double rh);
GridVector operator*(double lh, GridVector rh);
std::ostream& operator<<(std::ostream& os, GridVector gv);
bool operator==(GridVector lh, GridVector rh);

/**************************************************************************************/
// PIECE ENUM 

enum Piece {
    PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING, PIECE_NULL
};

std::ostream& operator<<(std::ostream& os, Piece t);

/**************************************************************************************/
// PLAYER ENUM 

enum Player {
    WHITE, BLACK, PLAYER_NULL
};

std::ostream& operator<<(std::ostream& os, Player t);
Player operator!(Player p);

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

std::ostream& operator<<(std::ostream& os, Move mv);
bool operator==(Move lh, Move rh);

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
std::ostream& operator<<(std::ostream& os, MoveCallback t);

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
