/* Chess consol */
#pragma once

#include "chess.hpp"
#include <iostream>
#include <string>
#include <unordered_map>

class ChessConsol
{

private:

    Board* board;

    bool running = false;

    std::unordered_map<char, int> letterToNum;

public:

    ChessConsol();

    void launch(std::vector<std::string> preMoves);
    void terminate();
    MoveCallback execute(std::string input);

};

ChessConsol::ChessConsol()
{
    board = new Board;
    board->setup();

    // create dictionary for file letters to int numbers
    letterToNum['a'] = 0;
    letterToNum['b'] = 1;
    letterToNum['c'] = 2;
    letterToNum['d'] = 3;
    letterToNum['e'] = 4;
    letterToNum['f'] = 5;
    letterToNum['g'] = 6;
    letterToNum['h'] = 7;
}

void ChessConsol::launch(std::vector<std::string> preMoves)
{
    running = true;
    MoveCallback cb = SUCCESS;

    std::string input;

    std::cout << std::endl;
    std::cout << "=== WELCOME TO CHESS by W.Denny ===" << std::endl;
    std::cout << "Type moves in following format: index1 index2, e.g. a1 b1" << std::endl;
    std::cout << "Castling is as follows: oo for kingside, ooo for queenside" << std::endl;
    std::cout << "To resign, type resign; to quit, type quit" << std::endl << std::endl;

    while (running == true)
    {
        if (cb == SUCCESS) // after successful move, step the board system and display
        {
            board->display();
        }

        if (board->getStatus() != IN_PROGRESS)
        {
            std::cout << "Game over: " << board->getStatus() << std::endl;
            if (board->getStatus() == CHECKMATE)
            {
                std::cout << "Winner: " << board->getWinner() << std::endl;
            }
            terminate();
            break;
        }
        if (board->getCheck() != PLAYER_NULL)
        {
            std::cout << std::endl << board->getCheck() << " is in check " << std::endl;
        }

        // get input either from user or from premoves
        std::cout << std::endl << board->getPlayerToMove() << " to move: ";
        if (preMoves.size() == 0)
            std::getline(std::cin, input);
        else
        {
            input = preMoves[0];
            std::cout << preMoves[0];
            preMoves.erase(preMoves.begin());
        }
        std::cout << std::endl;

        // execute input
        cb = execute(input);

        if (cb != SUCCESS)
        {
            std::cout << "Invalid input: " << cb << std::endl;
        }
        else
        {
            std::cout << "=================================================" << std::endl;
            std::cout << std::endl;
        }
    }
}

void ChessConsol::terminate()
{
    running = false;
}

MoveCallback ChessConsol::execute(std::string input)
{   
    if (input == "quit")
    {
        terminate();
        return GAME_OVER;
    }
    if (input == "resign")
    {   
        return GAME_OVER;
    }
    else if (input == "oo")
    {
        return GAME_OVER;
    }
    else if (input == "ooo")
    {
        return GAME_OVER;
    }
    else // standard input (although must check)
    {
        if (input.length() != 5)
            return INVALID_INPUT;

        if (letterToNum.find(input.at(0)) == letterToNum.end()) // ensure first file letter valid
            return INVALID_INPUT;

        if (letterToNum.find(input.at(3)) == letterToNum.end()) // ensure second file letter valid
            return INVALID_INPUT;

        int file0 = letterToNum[input.at(0)];
        int rank0 = ((int) (input.at(1) - '0')) - 1;
        
        int file1 = letterToNum[input.at(3)];
        int rank1 = ((int) (input.at(4) - '0')) - 1;

        if (input.at(2) != ' ') // ensure separated by space, as required by form
            return INVALID_INPUT;

        MoveCallback cb = board->move({{file0, rank0}, {file1, rank1}}); // ensure move is valid in context of chess rules and piece locations
        return cb;
    }
}