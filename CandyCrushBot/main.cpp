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
#include "bots.hpp"



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
    generateData = false;
    
    
    
    std::unordered_map<std::string, std::string> gameMoveMap;
    
    if (generateData) {
        
        std::cout << allSwapsForGame(game).size() << std::endl;
        std::cout << game.cells.size() + 1 << std::endl;
        
        auto moveNumberMax = (game.getGameBoard().rows * (game.getGameBoard().rows-1) * 2);
        
        int moveCounts[moveNumberMax];
        for (auto i = 0; i < moveNumberMax; i++) {
            moveCounts[i] = 0;
        }
        
        for (auto i = 0; i < numberOfMoves; i++) {
            if (game.gameOver()) {
                game = createGame();
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
//                std::cout << std::endl;
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
        
        std::vector<std::string> bots = {tensorFlowBot.name};
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
