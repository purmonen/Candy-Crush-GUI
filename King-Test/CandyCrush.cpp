#include "CandyCrush.hpp"

CandyCrush::Cell CandyCrush::randomCell() {
    std::vector<CandyCrush::Cell> cells = {Green, Blue, Purple, Red, Yellow};
    return cells[rand() % cells.size()];
}

// When randomly generating new cells some of them will create matches that must be cleared after each move and when initializing the game
void CandyCrush::clearAllMatches(GameBoardChangeCallback callback) {
    
    // As long as doing nothing increases score we should keep doing it
    auto doNothingMove = GameBoard::CellSwapMove(GameBoard::CellPosition(0,0), GameBoard::CellPosition(0,0));
    while (performMove(doNothingMove, callback)) {}
}

bool CandyCrush::isLegalMove(GameBoard::CellSwapMove move) const {
    // Perform the move on a copy of the game
    auto game = *this;
    return game.performMove(move);
}

// Could have a more advanced score function where many matches are much more rewarded
int CandyCrush::scoreForMatches(int numberOfMatches) const {
    return numberOfMatches;
}

// Performs the move and returns where it was valid
bool CandyCrush::performMove(GameBoard::CellSwapMove move, GameBoardChangeCallback callback) {
    if (!gameBoard.areCellsAdjacent(move.from, move.to) && !(move.from == move.to)) {
        return false;
    }
    
    auto gameBoardChange = CandyCrushGameBoardChange(*this);
    gameBoardChange.gameBoardChange[move.from] = {move.to, gameBoard[move.to]};
    gameBoardChange.gameBoardChange[move.to] = {move.from, gameBoard[move.from]};
    
    // This allows the caller to see the game board changes done so far, which currently is just a swap
    if (callback != nullptr) {
        callback(gameBoardChange);
    }
    
    // Needed to calculate if the score did increase at the end
    const auto oldScore = score;
    
    // Move pieces
    gameBoard.swapCells(move);
    gameBoardChange = CandyCrushGameBoardChange(*this);
    
    // Horizontal matching
    for (auto row = 0; row < gameBoard.rows; row++) {
        int numberOfMatches = 0;
        auto lastCell = gameBoard[row][0];
        for (auto column = 0; column < gameBoard.columns; column++) {
            bool cellsMatched = lastCell == gameBoard[row][column];
            if (cellsMatched) {
                numberOfMatches++;
            }
            if ((!cellsMatched || column == gameBoard.rows -1 ) && numberOfMatches >= 3) {
                score += scoreForMatches(numberOfMatches);
                auto lastColumn = column;
                if (cellsMatched) {
                    lastColumn++;
                }
                
                for (auto matchedColumn = lastColumn-numberOfMatches; matchedColumn < lastColumn; matchedColumn++) {
                    gameBoardChange.removedCells.push_back({GameBoard::CellPosition(row, matchedColumn), gameBoard[row][matchedColumn]});
                }
                
                for (auto matchedColumn = lastColumn-numberOfMatches; matchedColumn < lastColumn; matchedColumn++) {
                    for (auto matchedRow = row-1; matchedRow >= 0; matchedRow--) {
                        gameBoardChange.gameBoardChange[GameBoard::CellPosition(matchedRow+1, matchedColumn)] = {GameBoard::CellPosition(matchedRow, matchedColumn), gameBoard[GameBoard::CellPosition(matchedRow, matchedColumn)]};
                    }
                    gameBoardChange.gameBoardChange[GameBoard::CellPosition(0, matchedColumn)] = {GameBoard::CellPosition(-1, matchedColumn), randomCell()};
                }
            }
            if (!cellsMatched) {
                numberOfMatches = 1;
            }
            
            lastCell = gameBoard[row][column];
        }
    }
    // Vertical matching
    for (auto column = 0; column < gameBoard.columns; column++) {
        int numberOfMatches = 0;
        auto lastCell = gameBoard[0][column];
        for (auto row = 0; row < gameBoard.rows; row++) {
            bool cellsMatched = lastCell == gameBoard[row][column];
            if (cellsMatched) {
                numberOfMatches++;
            }
            if ((!cellsMatched || row == gameBoard.rows-1) && numberOfMatches >= 3) {
                score += scoreForMatches(numberOfMatches);
                int prevRow = row-numberOfMatches;
                if (!cellsMatched) {
                    prevRow--;
                }
                
                for (auto i = prevRow+1; i <= prevRow+numberOfMatches; i++) {
                    gameBoardChange.removedCells.push_back({GameBoard::CellPosition(i, column), gameBoard[i][column]});
                }
                
                while (prevRow >= 0) {
                    gameBoardChange.gameBoardChange[GameBoard::CellPosition(prevRow+numberOfMatches, column)] = {GameBoard::CellPosition(prevRow, column), gameBoard[GameBoard::CellPosition(prevRow, column)]};
                    prevRow--;
                }
                
                for (auto i = 0; i < numberOfMatches; i++) {
                    gameBoardChange.gameBoardChange[GameBoard::CellPosition(i, column)] = {GameBoard::CellPosition(i-numberOfMatches, column), randomCell()};
                }
                
                int matchedRow = row;
                if (!cellsMatched) {
                    matchedRow--;
                }
                for (auto i = 0; i < numberOfMatches; i++) {
                    matchedRow--;
                }
            }
            
            if (!cellsMatched) {
                numberOfMatches = 1;
            }
            lastCell = gameBoard[row][column];
        }
    }
    
    for (auto row = 0; row < gameBoard.rows; row++) {
        for (auto column = 0; column < gameBoard.columns; column++) {
            gameBoard[row][column] = gameBoardChange.gameBoardChange[GameBoard::CellPosition(row, column)].second;
        }
    }
    
    // Let the caller see game board after changes
    if (callback != nullptr) {
        callback(gameBoardChange);
    }
    
    if (oldScore == score) {
        
        // If the move did not increase the score, it's not valid and the swap must be undone!
        gameBoardChange.gameBoardChange[move.from] = {move.to, gameBoard[move.to]};
        gameBoardChange.gameBoardChange[move.to] = {move.from, gameBoard[move.from]};
        
        // Let the caller see the swap back
        if (callback != nullptr) {
            callback(gameBoardChange);
        }
        gameBoard.swapCells(move);
        return false;
    }
    return true;
    
}

// Return the game state that will occur after a move has been made
CandyCrush CandyCrush::gameForMove(GameBoard::CellSwapMove move) const {
    auto gameCopy = *this;
    gameCopy.performMove(move);
    return gameCopy;
}

CandyCrush::CandyCrush() {
    clearAllMatches();
    score = 0;
//    gameBoard[0][1] = Purple;
//    //gameBoard[1][1] = Purple;
//    gameBoard[1][2] = Purple;
//    gameBoard[1][3] = Purple;
//    gameBoard[2][1] = Purple;
//    gameBoard[3][1] = Purple;
}

const CandyCrush::CandyCrushGameBoard& CandyCrush::getGameBoard() const {
    return gameBoard;
}

bool CandyCrush::operator==(const CandyCrush & game) const {
    return gameBoard == game.gameBoard;
}

int CandyCrush::getScore() const {
    return score;
}

bool CandyCrush::play(GameBoard::CellSwapMove move, GameBoardChangeCallback callback) {
    if (!gameOver()) {
        auto isMoveValid = performMove(move, callback);
        clearAllMatches(callback);
        return isMoveValid;
    }
    return false;
}

bool CandyCrush::gameOver() const {
    
    // Legal moves can theoretically be empty since new cells are completely randomly generated
    return numberOfSecondsLeft() <= 0 || legalMoves().empty();
}

int CandyCrush::numberOfSecondsLeft() const {
    auto currentTime = std::chrono::high_resolution_clock::now();
    int numberOfSecondsElapsed = (int)std::chrono::duration_cast<std::chrono::seconds>(currentTime-startTime).count();
    return numberOfSecondsElapsed < timeLimitInSeconds ? timeLimitInSeconds-numberOfSecondsElapsed : 0;
}

std::vector<GameBoard::CellSwapMove> CandyCrush::legalMoves() const {
    std::vector<GameBoard::CellSwapMove> moves;
    for (auto row = 0 ; row < gameBoard.rows; row++) {
        for (auto column = 0; column < gameBoard.columns; column++) {
            GameBoard::CellPosition cell(row, column);
            for (auto adjacentCell: gameBoard.adjacentCells(cell)) {
                if (isLegalMove(GameBoard::CellSwapMove(cell, adjacentCell))) {
                    moves.push_back(GameBoard::CellSwapMove(cell, adjacentCell));
                }
            }
        }
    }
    return moves;
}

std::ostream& operator<<(std::ostream& os, const CandyCrush& game) {
    os << "--------------------" << std::endl;
    os << "Score: " << game.getScore() << std::endl;
    std::unordered_map<CandyCrush::Cell, std::string> cellString {
        {CandyCrush::Green, "G"},
        {CandyCrush::Blue, "B"},
        {CandyCrush::Purple, "P"},
        {CandyCrush::Yellow, "Y"},
        {CandyCrush::Red, "R"},
        //{CandyCrush::Purple, "!"}
    };
    
    std::cout << std::endl;
    
    auto& gameBoard = game.getGameBoard();
    for (auto i = 0; i < gameBoard.rows; i++) {
        for (auto j = 0; j < gameBoard.columns; j++) {
            os << cellString[gameBoard[i][j]] << " ";
        }
        os << std::endl;
    }
    std::cout << std::endl;
    return os;
}
