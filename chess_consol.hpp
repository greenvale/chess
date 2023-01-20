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
    bool execute(Player player, std::string input);

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
    bool success = true;

    Player player = WHITE;
    std::string input;

    std::cout << std::endl;
    std::cout << "=== WELCOME TO CHESS by W.Denny ===" << std::endl;
    std::cout << "Type moves in following format: index1 index2, e.g. a1 b1" << std::endl;
    std::cout << "Castling is as follows: oo for kingside, ooo for queenside" << std::endl;
    std::cout << "To resign, type resign; to quit, type quit" << std::endl << std::endl;

    while (running == true)
    {
        if (success == true) // after successful move, step the board system and display
        {
            board->step();
            board->display();
        }

        std::cout << std::endl << player << " to move: ";
        if (preMoves.size() == 0)
            std::getline(std::cin, input);
        else
        {
            input = preMoves[0];
            std::cout << preMoves[0];
            preMoves.erase(preMoves.begin());
        }
        std::cout << std::endl;

        if (input == "quit")
        {
            terminate();
        }
        else 
        {
            success = execute(player, input);

            if (success == true)
            {
                //std::cout << "Successful move" << std::endl;
            }
            else
            {
                std::cout << "Invalid input" << std::endl;
            }

            if (success == true)
            {
                if (player == WHITE)
                {
                    player = BLACK;
                }
                else
                {
                    player = WHITE;
                }

                std::cout << "=================================================" << std::endl;
                std::cout << std::endl;
            }
        } 
    }
}

void ChessConsol::terminate()
{
    running = false;
}

bool ChessConsol::execute(Player player, std::string input)
{   
    if (input == "resign")
    {
        
    }
    else if (input == "oo")
    {

    }
    else if (input == "ooo")
    {

    }
    else // standard input (although must check)
    {
        if (input.length() != 5)
            return false;

        if (letterToNum.find(input.at(0)) == letterToNum.end()) // ensure first file letter valid
            return false;

        if (letterToNum.find(input.at(3)) == letterToNum.end()) // ensure second file letter valid
            return false;

        int file0 = letterToNum[input.at(0)];
        int rank0 = ((int) (input.at(1) - '0')) - 1;
        
        int file1 = letterToNum[input.at(3)];
        int rank1 = ((int) (input.at(4) - '0')) - 1;

        if ((rank0 < 0) || (rank0 > 7)) // ensure first rank valid
            return false;

        if ((rank1 < 0) || (rank1 > 7)) // ensure second rank valid
            return false;

        if (input.at(2) != ' ') // ensure separated by space, as required by form
            return false;

        bool validation = board->move({{file0, rank0}, {file1, rank1}}); // ensure move is valid in context of chess rules and piece locations

        if (validation == false)
        {
            return false;
        }
    }
    return true;
}