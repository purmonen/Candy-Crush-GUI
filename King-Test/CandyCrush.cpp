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
    gameBoardChange = CandyCrushGameBoardChange(*this);
    
    // Horizontal matching – look for 3 or more consecutive columns on the same row with the same color
    for (auto row = 0; row < gameBoard.rows; row++) {
        int numberOfMatches = 0;
        auto lastCell = gameBoard[row][0];
        for (auto column = 0; column < gameBoard.columns; column++) {
            auto currentCell = gameBoard[row][column];
            bool cellsMatched = lastCell == currentCell;
            if (cellsMatched) {
                numberOfMatches++;
            }
            
            if (numberOfMatches >= 3 && (!cellsMatched || column == gameBoard.rows -1 )) {
                score += scoreForMatches(numberOfMatches);
                auto lastColumnThatMatched = cellsMatched ? column : column-1;
                
                for (auto matchedColumn = lastColumnThatMatched-numberOfMatches+1; matchedColumn <= lastColumnThatMatched; matchedColumn++) {
                    gameBoardChange.removedCells.push_back({{row, matchedColumn}, gameBoard[row][matchedColumn]});
                    
                    // Move existing cells down
                    for (auto matchedRow = row-1; matchedRow >= 0; matchedRow--) {
                        gameBoardChange.gameBoardChange[{matchedRow+1, matchedColumn}] = {{matchedRow, matchedColumn}, gameBoard[matchedRow][matchedColumn]};
                    }
                    // Create new cell
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
            bool cellsMatched = lastCell == currentCell;
            if (cellsMatched) {
                numberOfMatches++;
            }
            if (numberOfMatches >= 3 && (!cellsMatched || row == gameBoard.rows-1)) {
                score += scoreForMatches(numberOfMatches);
                int firstPreviousRowNotMatching = cellsMatched ? row-numberOfMatches : row-numberOfMatches-1;
                
                for (auto i = firstPreviousRowNotMatching+1; i <= firstPreviousRowNotMatching+numberOfMatches; i++) {
                    gameBoardChange.removedCells.push_back({{i, column}, gameBoard[i][column]});
                }
                
                // Move down already existing cells
                while (firstPreviousRowNotMatching >= 0) {
                    gameBoardChange.gameBoardChange[{firstPreviousRowNotMatching+numberOfMatches, column}] = {{firstPreviousRowNotMatching, column}, gameBoard[firstPreviousRowNotMatching][column]};
                    firstPreviousRowNotMatching--;
                }
                
                // Add new cells at the top
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
