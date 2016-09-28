
#ifndef bots_hpp
#define bots_hpp

#include <vector>
#include <assert.h>
#include "GameBoard.hpp"
#include <vector>
#include <sstream>
#include <ctime>
#include <unordered_map>
#include <iostream>

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

std::string gameToLine(const CandyCrush &game) {
    std::stringstream string;
    
    auto& gameBoard = game.getGameBoard();
    for (auto cell: game.cells) {
        for (auto row = 0; row  < gameBoard.rows; row++) {
            for (auto column = 0; column  < gameBoard.columns; column++) {
                string << (gameBoard[row][column] == cell ? 1 : 0) << " ";
            }
        }
        
        string << std::endl;
    }
    return string.str();
}

std::string legalMovesToLine(const CandyCrush &game) {
    std::stringstream string;
    
    
    auto& gameBoard = game.getGameBoard();
    auto legalMoves = game.legalMoves();
        for (auto row = 0; row  < gameBoard.rows; row++) {
            for (auto column = 0; column  < gameBoard.columns; column++) {
                bool isLegalMove = false;
                auto cellPosition = GameBoard::CellPosition(row, column);
                for (auto move: legalMoves) {
                    if (move.from == cellPosition || move.to == cellPosition) {
                        isLegalMove = true;
                        break;
                    }
                }
                string << (isLegalMove ? 1 : 0) << " ";
            }
        }
    string << std::endl;
    return string.str();
}

std::string moveToLine(size_t moveNumber, const CandyCrush &game) {
    std::stringstream string;
    string << moveNumber;
    //    auto moveNumberMax = (game.getGameBoard().rows * (game.getGameBoard().rows-1) * 2);
    //    for (auto i = 0; i < moveNumberMax; i++) {
    //        string << (i == moveNumber ? 1 : 0) << " ";
    //    }
    return string.str();
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

class DeterministicBot {
public:
    GameBoard::CellSwapMove selectMove(CandyCrush game) {
        auto legalMoves = game.legalMoves();
        int maxRow = 0;
        int maxColumn = 0;
        GameBoard::CellSwapMove maxMove = legalMoves[0];
        
        for (auto move: legalMoves) {
            int row = std::max(move.from.row, move.to.row);
            int column = std::max(move.from.column, move.to.column);
            if (row > maxRow || (row == maxRow && column >= maxColumn)) {
                maxRow = row;
                maxColumn = column;
                maxMove = move;
            }
        }
        return maxMove;
    }
};


class WolframBot {
public:
    GameBoard::CellSwapMove selectMove(CandyCrush game) {
        std::string command = "/Applications/Mathematica.app/Contents/MacOS/WolframKernel -script /Users/samipurmonen/Desktop/ai-bot/predict2.m  '" + gameToLine(game) + "'";
        
        std::cout << command << std::endl;
        //command = "echo $USER";
        auto python_output = exec(command.c_str());
        std::cout << python_output;
        
        
        std::vector<int> moveProbabilities(allSwapsForGame(game).size());
        size_t move = 0;
        int probability = 0;
        std::istringstream s2(python_output);
        while (s2 >> move) {
            s2 >> probability;
            moveProbabilities[move] = probability;
        }
        
        std::vector<int> sortedMoves(moveProbabilities.size());
        for (auto i = 0; i < sortedMoves.size(); i++) {
            sortedMoves[i] = i;
        }
        
        std::sort(sortedMoves.begin(), sortedMoves.end(), [&](auto x, auto y){
            return moveProbabilities[x] > moveProbabilities[y];
        });
        
        for (auto i = 0; i < sortedMoves.size(); i++) {
            auto move = sortedMoves[i];
            std::cout << i << ". Move " << move << " " << moveProbabilities[move] << " " << game.isLegalMove(cellSwapMoveForNumber(move, game)) << std::endl;
        }
        
        for (int i = 0; i < sortedMoves.size(); i++) {
            auto move = sortedMoves[i];
            if (game.isLegalMove(cellSwapMoveForNumber(move, game))) {
                std::cout << "Move found on index " << i << std::endl;
                return cellSwapMoveForNumber(move, game);
            }
        }
        throw "Error: move not found but there must be one!";
        
    }
    
    const std::string name = "WolframBot";
};


class TensorFlowBot {
public:
    GameBoard::CellSwapMove selectMove(CandyCrush game) {
        std::string command = "/Library/Frameworks/Python.framework/Versions/3.4/bin/python3 /Users/Sami/Desktop/ai-bot/cnn.py predict  '" + gameToLine(game) + "'";
        
        std::cout << command << std::endl;
        auto python_output = exec(command.c_str());
        std::cout << python_output;
        
        
        std::vector<int> moveProbabilities(allSwapsForGame(game).size());
        size_t move = 0;
        int probability = 0;
        std::istringstream s2(python_output);
        while (s2 >> move) {
            s2 >> probability;
            moveProbabilities[move] = probability;
        }
        
        std::vector<int> sortedMoves(moveProbabilities.size());
        for (auto i = 0; i < sortedMoves.size(); i++) {
            sortedMoves[i] = i;
        }
        
        std::sort(sortedMoves.begin(), sortedMoves.end(), [&](auto x, auto y){
            return moveProbabilities[x] > moveProbabilities[y];
        });
        
        for (auto i = 0; i < sortedMoves.size(); i++) {
            auto move = sortedMoves[i];
            std::cout << i << ". Move " << move << " " << moveProbabilities[move] << " " << game.isLegalMove(cellSwapMoveForNumber(move, game)) << std::endl;
        }
        
        for (int i = 0; i < sortedMoves.size(); i++) {
            auto move = sortedMoves[i];
            if (game.isLegalMove(cellSwapMoveForNumber(move, game))) {
                std::cout << "Move found on index " << i << std::endl;
                return cellSwapMoveForNumber(move, game);
            }
        }
        throw "Error: move not found but there must be one!";
        
    }

    
    const std::string name = "TensorFlowBot";
};

#endif /* bots_hpp */
