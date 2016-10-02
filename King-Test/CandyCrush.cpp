#include "CandyCrush.hpp"
#include "assert.h"

CandyCrush::Cell CandyCrush::randomCell() {
    return cells[rand() % numberOfCandies];
}

// When randomly generating new cells some of them will create matches that must be cleared after each move and when initializing the game
void CandyCrush::clearAllMatches(GameBoardChangeCallback callback) {
    
    // As long as doing nothing increases score we should keep doing it
    auto doNothingMove = GameBoard::CellSwapMove(GameBoard::CellPosition(0,0), GameBoard::CellPosition(0,0));
    while (performMove(doNothingMove, callback)) {}
}

int CandyCrush::numberOfMatchesForMove(GameBoard::CellSwapMove move) const {
    auto gameBoard = this->gameBoard;
    gameBoard.swapCells(move);
    
    auto totalMatches = 0;
    
    auto minimumRequiredNumberOfMatches = 3;
    
    for (auto cellPosition: {move.from, move.to}) {
        auto horizontalMatches = 0;
        auto verticalMatches = 0;
        
        for (int row = cellPosition.row-1; row >= 0; row--) {
            if (gameBoard[row][cellPosition.column] == gameBoard[cellPosition]) {
                verticalMatches++;
            } else {
                break;
            }
        }
        for (int row = cellPosition.row+1; row < gameBoard.rows; row++) {
            if (gameBoard[row][cellPosition.column] == gameBoard[cellPosition]) {
                verticalMatches++;
            } else {
                break;
            }
        }
        
        for (int column = cellPosition.column-1; column >= 0; column--) {
            if (gameBoard[cellPosition.row][column] == gameBoard[cellPosition]) {
                horizontalMatches++;
            } else {
                break;
            }
        }
        for (int column = cellPosition.column+1; column < gameBoard.columns; column++) {
            if (gameBoard[cellPosition.row][column] == gameBoard[cellPosition]) {
                horizontalMatches++;
            } else {
                break;
            }
        }
        
        int matches = 1;
        if (horizontalMatches >= minimumRequiredNumberOfMatches - 1) {
            matches += horizontalMatches;
        }
        if (verticalMatches >= minimumRequiredNumberOfMatches - 1) {
            matches +=  verticalMatches;
        }
        if (matches >= minimumRequiredNumberOfMatches) {
            totalMatches += matches;
        }
    }
    return totalMatches;
}

bool CandyCrush::isLegalMoveFast(GameBoard::CellSwapMove move) const {
    if (!gameBoard.isCellValid(move.from) || !gameBoard.isCellValid(move.to)) {
        return false;
    }
    
    auto gameBoard = this->gameBoard;
    gameBoard.swapCells(move);
    
    
    for (auto fromPosition: {move.from, move.to}) {
        //auto toPosition = fromPosition == move.from ? move.to : move.from;
        for (auto direction: {GameBoard::Direction::Up, GameBoard::Direction::Right, GameBoard::Direction::Down, GameBoard::Direction::Left}) {
            auto firstNeighborCell = fromPosition.cellAtDirection(direction);
            auto secondNeighborCell = firstNeighborCell.cellAtDirection(direction);
            
            
            if (gameBoard.isCellValid(firstNeighborCell) && gameBoard.isCellValid(secondNeighborCell)) {
                if (gameBoard[fromPosition] == gameBoard[firstNeighborCell] && gameBoard[firstNeighborCell] == gameBoard[secondNeighborCell]) {
                    return true;
                }
            }
        }
        auto upCell = fromPosition.cellAtDirection(GameBoard::Up);
        auto downCell = fromPosition.cellAtDirection(GameBoard::Down);
        auto leftCell = fromPosition.cellAtDirection(GameBoard::Left);
        auto rightCell = fromPosition.cellAtDirection(GameBoard::Right);
        
        if (gameBoard.isCellValid(upCell) && gameBoard.isCellValid(downCell) && gameBoard[upCell] == gameBoard[fromPosition] && gameBoard[upCell] == gameBoard[downCell]) {
            return true;
        }
        if (gameBoard.isCellValid(leftCell) && gameBoard.isCellValid(rightCell) && gameBoard[leftCell] == gameBoard[fromPosition] && gameBoard[leftCell] == gameBoard[rightCell]) {
            return true;
        }
    }
    return false;
}

bool CandyCrush::isLegalMove(GameBoard::CellSwapMove move) const {
    
    
    
    // Perform the move on a copy of the game and returns whether it was valid or not
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
    
    // This resets the gameBoardChange so the swap won't be seen as a change by the caller next time
    gameBoardChange = CandyCrushGameBoardChange(*this);
    
    // Horizontal matching – look for 3 or more consecutive columns on the same row with the same color
    for (auto row = 0; row < gameBoard.rows; row++) {
        int numberOfMatches = 0;
        auto lastCell = gameBoard[row][0];
        for (auto column = 0; column < gameBoard.columns; column++) {
            auto currentCell = gameBoard[row][column];
            auto cellsMatched = lastCell == currentCell;
            if (cellsMatched) {
                numberOfMatches++;
            }
            
            // We should keep looking for more matches unless we find a non-matching cell or are at the end of the board
            if (numberOfMatches >= 3 && (!cellsMatched || column == gameBoard.rows -1 )) {
                score += scoreForMatches(numberOfMatches);
                auto lastColumnThatMatched = cellsMatched ? column : column-1;
                
                for (auto matchedColumn = lastColumnThatMatched-numberOfMatches+1; matchedColumn <= lastColumnThatMatched; matchedColumn++) {
                    
                    // Matched columns will be removed
                    GameBoard::CellPosition removedCellPosition = {row, matchedColumn};
                    gameBoardChange.removedCells.push_back({removedCellPosition, gameBoard[removedCellPosition]});
                    
                    // Move existing cells above down one step
                    for (auto matchedRow = row-1; matchedRow >= 0; matchedRow--) {
                        GameBoard::CellPosition cellPosition = {matchedRow, matchedColumn};
                        GameBoard::CellPosition newCellPosition = {matchedRow+1, matchedColumn};
                        gameBoardChange.gameBoardChange[newCellPosition] = {cellPosition, gameBoard[cellPosition]};
                    }
                    
                    // Create new cell at the top of the board and say it will come above the board (-1)
                    gameBoardChange.gameBoardChange[{0, matchedColumn}] = {{-1, matchedColumn}, randomCell()};
                }
            }
            if (!cellsMatched) {
                numberOfMatches = 1;
            }
            
            lastCell = gameBoard[row][column];
        }
    }
    // Vertical matching – look for 3 or more consecutive rows in same column with the same color
    for (auto column = 0; column < gameBoard.columns; column++) {
        int numberOfMatches = 0;
        auto lastCell = gameBoard[0][column];
        for (auto row = 0; row < gameBoard.rows; row++) {
            auto currentCell = gameBoard[row][column];
            auto cellsMatched = lastCell == currentCell;
            if (cellsMatched) {
                numberOfMatches++;
            }
            if (numberOfMatches >= 3 && (!cellsMatched || row == gameBoard.rows-1)) {
                score += scoreForMatches(numberOfMatches);
                int firstPreviousRowNotMatching = cellsMatched ? row-numberOfMatches : row-numberOfMatches-1;
                
                for (auto i = firstPreviousRowNotMatching+1; i <= firstPreviousRowNotMatching+numberOfMatches; i++) {
                    GameBoard::CellPosition removedCellPosition = {i, column};
                    gameBoardChange.removedCells.push_back({removedCellPosition, gameBoard[removedCellPosition]});
                }
                
                // Move down already existing cells from above
                while (firstPreviousRowNotMatching >= 0) {
                    GameBoard::CellPosition cellPosition = {firstPreviousRowNotMatching, column};
                    GameBoard::CellPosition newCellPosition = {firstPreviousRowNotMatching+numberOfMatches, column};
                    gameBoardChange.gameBoardChange[newCellPosition] = {cellPosition, gameBoard[cellPosition]};
                    firstPreviousRowNotMatching--;
                }
                
                // Add new cells at the top and say that they will come from above the board
                for (auto i = 0; i < numberOfMatches; i++) {
                    gameBoardChange.gameBoardChange[{i, column}] = {{i-numberOfMatches, column}, randomCell()};
                }
            }
            
            if (!cellsMatched) {
                numberOfMatches = 1;
            }
            lastCell = gameBoard[row][column];
        }
    }
    
    // Apply all changes to the actual game board now
    for (auto row = 0; row < gameBoard.rows; row++) {
        for (auto column = 0; column < gameBoard.columns; column++) {
            gameBoard[row][column] = gameBoardChange.gameBoardChange[{row, column}].second;
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
    // The randomized board may have matching cells already – must be removed!
    clearAllMatches();
    score = 0;
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
        if (isMoveValid) {
            numberOfMovesLeft--;
        }
        return isMoveValid;
    }
    return false;
}

bool CandyCrush::gameOver() const {
    return legalMoves().empty() || numberOfMovesLeft < 1;
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
                
                
                //                assert(isLegalMove(GameBoard::CellSwapMove(cell, adjacentCell)) == isLegalMoveFast(GameBoard::CellSwapMove(cell, adjacentCell)));
                if (isLegalMoveFast(GameBoard::CellSwapMove(cell, adjacentCell))) {
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
        //        {CandyCrush::Yellow, "Y"},
        //        {CandyCrush::Red, "R"},
        //{CandyCrush::Purple, "!"}
    };
    
    os << std::endl;
    
    auto& gameBoard = game.getGameBoard();
    for (auto i = 0; i < gameBoard.rows; i++) {
        for (auto j = 0; j < gameBoard.columns; j++) {
            os << cellString[gameBoard[i][j]] << " ";
        }
        os << std::endl;
    }
    os << std::endl;
    return os;
}
