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
#include <ctime>
#include <unordered_map>

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
    for (auto row = 0; row  < gameBoard.rows; row++) {
        for (auto column = 0; column  < gameBoard.columns; column++) {
            string << gameBoard[row][column] << " ";
        }
    }
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
        return cellSwapMoveForNumber(0, game);
        std::string command = "/Users/samipurmonen/Desktop/ai-bot/ai_venv/bin/python3 /Users/samipurmonen/Desktop/ai-bot/candy_bot.py predict  '" + gameToLine(game) + "' -hidden_layers 3";
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
    
    const std::string name = "TensorFlowBot";
};


CandyCrush createGame() {
    return CandyCrush(8, 8, 60, 5);
}


int main(int argc, const char * argv[]) {
    
    bool generateData = false;
    int timeLimit = 10*60;
    int numberOfMoves = 10;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--generate_data") {
            generateData = true;
        }
        if (arg == "-time_limit") {
            if (i+1 < argc) {
                timeLimit = std::atoi(argv[i+1]);
            } else {
                std::cout << "Missing time limit" << std::endl;
            }
        }
        if (arg == "-number_of_moves") {
            if (i+1 < argc) {
                numberOfMoves = std::atoi(argv[i+1]);
            } else {
                std::cout << "Missing number of moves" << std::endl;
            }
        }
    }
    
    CandyCrush game = createGame();
//    generateData = true;
    
    
    
    std::unordered_map<std::string, std::string> gameMoveMap;
    
    if (generateData) {
        
        std::cout << allSwapsForGame(game).size() << std::endl;
        
        auto moveNumberMax = (game.getGameBoard().rows * (game.getGameBoard().rows-1) * 2);
        
        int moveCounts[moveNumberMax];
        for (auto i = 0; i < moveNumberMax; i++) {
            moveCounts[i] = 0;
        }
        
        for (auto i = 0; i < numberOfMoves; i++) {
            CandyCrush game = createGame();
            if (game.gameOver()) {
                continue;
            }
            auto move = DeterministicBot().selectMove(game);
            
            auto moveNumber = numberForCellSwapMove(move, game);
            
            
            assert(moveNumber < moveNumberMax);
            assert(moveNumber >= 0);
            
            std::string gameString = gameToLine(game);
            std::string moveString = moveToLine(moveNumber, game);
            
            if (gameMoveMap.find(gameString) == gameMoveMap.end()) {
                gameMoveMap[gameString] = moveString;
                moveCounts[moveNumber]++;
                std::cout << gameString;
                std::cout << std::endl;
                std::cout << moveString;
                std::cout << std::endl;
            } else {
                assert(gameMoveMap[gameString] == moveString);
            }
            game.play(move);
            
        }
        
        
//         for (auto i = 0; i < moveNumberMax; i++) {
//         std::cout << i << ": " << moveCounts[i] / double(numberOfMoves) * 100 << std::endl;
//         }
        
        
        /*
         while (!game.gameOver()) {
         auto move = DeterministicBot().selectMove(game);
         
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
         */
    } else {
        std::cout << "Running tensor flow!" << std::endl;
        
        
        struct BotRun {
            int numberOfMoves = 0;
            int numberOfValidMoves = 0;
            int correctMoves = 0;
        };
        
        auto tensorFlowBot = TensorFlowBot();
        auto wolframBot = WolframBot();
        
        std::unordered_map<std::string, BotRun> botRuns;
        botRuns[tensorFlowBot.name] = BotRun();
        botRuns[wolframBot.name] = BotRun();
        
        auto moveForBot = [&](std::string botName, const CandyCrush &game) { return botName == tensorFlowBot.name ? tensorFlowBot.selectMove(game) : wolframBot.selectMove(game); };
        
        std::vector<std::string> bots = {wolframBot.name};
        auto i = 0;
        while (!game.gameOver()) {
            game = createGame();
            
            auto gameString = gameToLine(game);
            
            if (gameMoveMap.find(gameString) != gameMoveMap.end()) {
                continue;
            }
            gameMoveMap[gameString] = "yolo";
            
            
            const auto move = DeterministicBot().selectMove(game);
            
            std::cout << "<<<<<<<<<< " << "Real World" << " >>>>>>>>>>>" << std::endl;
            std::cout << gameToLine(game) << std::endl;
            std::cout << "Number of valid moves: " << game.legalMoves().size() << std::endl;
            std::cout << "Selected move: " << numberForCellSwapMove(move, game) << std::endl;
            std::cout << std::endl;
            
            for (auto bot: bots) {
                
                std::cout << "<<<<<<<<<< " << bot << " >>>>>>>>>>>" << std::endl;
                
                auto predicted_move = moveForBot(bot, game);
                
                if (move == predicted_move) {
                    botRuns[bot].correctMoves++;
                }
                
                std::cout << "Predicted move: " << numberForCellSwapMove(predicted_move, game) << std::endl;
                
                auto legalMoves = game.legalMoves();
                botRuns[bot].numberOfMoves++;
                if (std::find(legalMoves.begin(), legalMoves.end(), predicted_move) != legalMoves.end()) {
                    botRuns[bot].numberOfValidMoves++;
                }
                std::cout << "Valid moves " << botRuns[bot].numberOfValidMoves << " / " << botRuns[bot].numberOfMoves << std::endl;
                std::cout << "Correct moves " << botRuns[bot].correctMoves << " / " << botRuns[bot].numberOfMoves << std::endl;
                std::cout << std::endl;
            }
            
            game.play(move);
            
            
            if (i > 300) {
                break;
            }
        }
        
//        std::cout << "Valid moves " << numberOfValidMoves / numberOfMoves * 100 << std::endl;
//        std::cout << "Correct moves " << correctMoves / numberOfMoves * 100 << std::endl;
    }
    return 0;
}
