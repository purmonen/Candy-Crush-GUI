//
//  main.cpp
//  CandyCrushBot
//
//  Created by Sami Purmonen on 14/09/16.
//  Copyright © 2016 Sami Purmonen. All rights reserved.
//

#include <iostream>
#include "CandyCrush.hpp"
#include <assert.h>
#include "GameBoard.hpp"
#include <vector>
#include <sstream>

/*
size_t numberForMove(const GameBoard::CellSwapMove &move, const CandyCrush &game) {
    auto row = std::min(move.from.row, move.to.row);
    auto column = std::min(move.from.column, move.to.column);
    
    auto columns = game.getGameBoard().columns;
    auto rows = game.getGameBoard().rows;
    if (move.from.row == move.to.row) {
        columns--;
        return row * columns + column % columns;
    } else {
        rows--;
        return rows * columns + row * columns + column % columns;
    }

}
 */

/*
 auto oldMoveNumber = 0;
 
 for (auto row = 0; row < game.getGameBoard().rows; row++) {
 for (auto column = 0; column < game.getGameBoard().columns-1; column++) {
 auto move = GameBoard::CellSwapMove(GameBoard::CellPosition(row, column), GameBoard::CellPosition(row, column+1));
 auto moveNumber = numberForMove(move, game);
 assert(oldMoveNumber == 0 || oldMoveNumber+1 == moveNumber);
 std::cout << moveNumber << std::endl;
 }
 }
 
 for (auto row = 0; row < game.getGameBoard().rows-1; row++) {
 for (auto column = 0; column < game.getGameBoard().columns; column++) {
 auto move = GameBoard::CellSwapMove(GameBoard::CellPosition(row, column), GameBoard::CellPosition(row+1, column));
 auto moveNumber = numberForMove(move, game);
 assert(oldMoveNumber == 0 || oldMoveNumber+1 == moveNumber);
 std::cout << moveNumber << std::endl;
 }
 }
 */

std::vector<GameBoard::CellSwapMove> allSwapsForGame(const CandyCrush &game) {
    std::vector<GameBoard::CellSwapMove> swaps;
    
    for (auto row = 0; row < game.getGameBoard().rows; row++) {
        for (auto column = 0; column < game.getGameBoard().columns-1; column++) {
            auto move = GameBoard::CellSwapMove(GameBoard::CellPosition(row, column), GameBoard::CellPosition(row, column+1));
            swaps.push_back(move);
        }
    }
    
    for (auto row = 0; row < game.getGameBoard().rows-1; row++) {
        for (auto column = 0; column < game.getGameBoard().columns; column++) {
            auto move = GameBoard::CellSwapMove(GameBoard::CellPosition(row, column), GameBoard::CellPosition(row+1, column));
            swaps.push_back(move);
        }
    }
    
    return swaps;
}

size_t numberForCellSwapMove(const GameBoard::CellSwapMove &move, const CandyCrush &game) {
    auto swaps = allSwapsForGame(game);
    
    for (int i = 0; i < swaps.size(); i++) {
        if (swaps[i] == move) {
            return i;
        }
    }
    throw std::string("BAD MOVE!!!");
}

GameBoard::CellSwapMove cellSwapMoveForNumber(size_t number, const CandyCrush &game) {
    return allSwapsForGame(game)[number];
}


std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

class RandomBot {
public:
    GameBoard::CellSwapMove selectMove(CandyCrush game) {
        return game.legalMoves()[arc4random() % game.legalMoves().size()];
    }
};

std::string stringForGame(const CandyCrush &game) {
    std::stringstream ss;
    auto gameBoard = game.getGameBoard();
    for (auto row = 0; row  < gameBoard.rows; row++) {
        for (auto column = 0; column  < gameBoard.columns; column++) {
            ss << gameBoard[row][column] << " ";
        }
    }
    return ss.str();
}

class TensorFlowBot {
public:
    GameBoard::CellSwapMove selectMove(CandyCrush game) {
        std::string command = "python3 /Users/samipurmonen/Desktop/ai-bot/candy_bot.py predict '" + stringForGame(game) + "'";
        auto python_output = exec(command.c_str());
        
        double maxProbability = 0.0;
        size_t maxMove = 0;
        size_t move = 0;
        int probability = 0;
        
        std::istringstream s2(python_output);
        std::vector<int> v;
        while (s2 >> probability) {
            //std::cout << probability << std::endl;
            if (probability > maxProbability) {
                maxProbability = probability;
                maxMove = move;
                
            }
            move++;
        }
        
        return cellSwapMoveForNumber(maxMove, game);
    }
};

int main(int argc, const char * argv[]) {
    CandyCrush game(100);

    
    bool generate_data = true;
    
    if (generate_data) {
        while (!game.gameOver()) {
            auto move = RandomBot().selectMove(game);
            
            game.play(move);
            
            auto gameBoard = game.getGameBoard();
            for (auto row = 0; row  < gameBoard.rows; row++) {
                for (auto column = 0; column  < gameBoard.columns; column++) {
                    std::cout << gameBoard[row][column] << " ";
                }
            }
            std::cout << std::endl;
            
            auto moveNumber = numberForCellSwapMove(move, game);
            auto moveNumberMax = (game.getGameBoard().rows * (game.getGameBoard().rows-1) * 2);
            
            assert(moveNumber < moveNumberMax);
            assert(moveNumber >= 0);
            
            for (auto i = 0; i < moveNumberMax; i++) {
                std::cout << (i == moveNumber ? 1 : 0) << " ";
            }
            std::cout << std::endl;
        }
    } else {
        std::cout << "Running tensor flow!" << std::endl;
        
        int numberOfMoves = 0;
        int numberOfValidMoves = 0;
        while (!game.gameOver()) {
            auto move = TensorFlowBot().selectMove(game);
            std::cout << stringForGame(game) << std::endl;
            std::cout << "Selected move: " << numberForCellSwapMove(move, game) << std::endl;
            
            auto legalMoves = game.legalMoves();
            numberOfMoves++;
            if (std::find(legalMoves.begin(), legalMoves.end(), move) != legalMoves.end()) {
                game.play(move);
                numberOfValidMoves++;
            } else {
                std::cout << "Invalid move" << std::endl;
                game.play(RandomBot().selectMove(game));
            }
            
            std::cout << "Valid moves " << numberOfValidMoves << " / " << numberOfMoves << std::endl;
        }
    }
    return 0;
}
