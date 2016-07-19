#ifndef CandyCrush_hpp
#define CandyCrush_hpp

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "GameBoard.hpp"
#include <chrono>

struct CandyCrushGameBoardChange;



class CandyCrush {
public:
    enum Cell {Green, Blue, Purple, Red, Yellow};
    typedef GameBoard::GameBoard<8, 8, CandyCrush::Cell> CandyCrushGameBoard;
    typedef std::function<void(CandyCrushGameBoardChange)> GameBoardChangeCallback;
    
private:
//    int numberOfMovesLeft = 100;
    CandyCrushGameBoard gameBoard = CandyCrushGameBoard([](auto rows, auto columns) {
        std::vector<CandyCrush::Cell> cells = {Green, Blue, Purple, Red, Yellow};
        return cells[rand() % cells.size()];
    });
    
    int timeLimitInSeconds = 3;
    
    
    int score = 0;
    Cell randomCell();
    void clearAllMatches(GameBoardChangeCallback callback = nullptr);
    
    int scoreForMatches(int numberOfMatches);
    bool performMove(GameBoard::CellSwapMove move, GameBoardChangeCallback callback = nullptr);
public:
    CandyCrush();
    const CandyCrushGameBoard& getGameBoard() const;
    bool isLegalMove(GameBoard::CellSwapMove move) const;
    bool operator==(const CandyCrush & game) const;
    int getScore() const;
    int getNumberOfMovesLeft() const;
    int numberOfSecondsLeft() const;
    bool play(GameBoard::CellSwapMove move, GameBoardChangeCallback callback = nullptr);
    bool gameOver() const;
    std::vector<GameBoard::CellSwapMove> legalMoves() const;
    
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
    
};



struct CandyCrushGameBoardChange {
    std::unordered_map<GameBoard::CellPosition, std::pair<GameBoard::CellPosition, CandyCrush::Cell>> gameBoardChange;
    std::vector<std::pair<GameBoard::CellPosition, CandyCrush::Cell>> removedCells;
    
    CandyCrushGameBoardChange(CandyCrush game) {
        for (auto row = 0; row < game.getGameBoard().rows; row++) {
            for (auto column = 0; column < game.getGameBoard().columns; column++) {
                gameBoardChange[GameBoard::CellPosition(row, column)] = {GameBoard::CellPosition(row, column), game.getGameBoard()[GameBoard::CellPosition(row, column)]};
            }
        }
    }
    
    
};

std::ostream& operator<<(std::ostream& os, const CandyCrush& game);

#endif /* CandyCrush_hpp */

