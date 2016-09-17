#ifndef GameBoard_hpp
#define GameBoard_hpp

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace GameBoard {
    
    enum Direction {Up, Right, Down, Left};
    
    struct CellPosition {
        int row = 0;
        int column = 0;
        
        CellPosition(int row, int column): row(row), column(column) {}
        CellPosition() {}
        
        CellPosition cellAtDirection(Direction direction) {
            switch (direction) {
                case Up: return CellPosition(row-1, column);
                case Right: return CellPosition(row, column+1);
                case Down: return CellPosition(row+1, column);
                case Left: return CellPosition(row, column-1);
            }
        }
        
        bool operator ==(const CellPosition& position) const {
            return row == position.row && column == position.column;
        }
    };
    
    std::ostream& operator<<(std::ostream& os, const CellPosition& cellPosition);
    
    struct CellSwapMove {
        CellPosition from;
        CellPosition to;
        
        CellSwapMove(CellPosition from, CellPosition to): from(from), to(to) {}
        
        bool operator ==(const CellSwapMove& move) const {
            return (from == move.from && to == move.to) || (from == move.to && to == move.from);
        }
    };
    
    std::ostream& operator<<(std::ostream& os, const CellSwapMove& move);
    
    template<typename CellType>
    class GameBoard {
    private:
        std::vector<std::vector<CellType>> gameBoard;
    public:
        size_t rows;
        size_t columns;
        
        GameBoard(const size_t rows, const size_t columns, const std::function<CellType(size_t, size_t)> defaultValueForCell): rows(rows), columns(columns) {
            for (auto row = 0; row < rows; row++) {
                gameBoard.push_back(std::vector<CellType>(columns));
            }
            
            performActionOnCell([&](auto row, auto column, auto& cell) {
                cell = defaultValueForCell(row, column);
            });
        }
        
        std::vector<CellType>& operator[](const size_t index) {
            return gameBoard[index];
        }
        
        
        const std::vector<CellType> operator[](const size_t index) const {
            return gameBoard[index];
        }
        
        
        CellType& operator[](const CellPosition cell) {
            return gameBoard[cell.row][cell.column];
        }
        
        const CellType& operator[](const CellPosition cell) const {
            return gameBoard[cell.row][cell.column];
        }
        
        void swapCells(const CellSwapMove cellSwapMove) {
            swapCells(cellSwapMove.from, cellSwapMove.to);
        }
        
        void swapCells(const CellPosition firstCell, const CellPosition secondCell) {
            std::swap((*this)[firstCell], (*this)[secondCell]);
        }
        
        bool isCellValid(CellPosition cell) const {
            return cell.row >= 0 && cell.row < rows && cell.column >= 0 && cell.column < columns;
        }
        
        bool areCellsAdjacent(CellPosition cell1, CellPosition cell2) const {
            if (!isCellValid(cell1)) {
                return false;
            }
            auto cells = adjacentCells(cell1);
            return std::find(cells.begin(), cells.end(), cell2) != cells.end();
        }
        
        std::vector<CellPosition> adjacentCells(CellPosition cell) const {
            std::vector<CellPosition> cells;
            for (auto direction: {Up, Right, Down, Left}) {
                auto adjacentCell = cell.cellAtDirection(direction);
                if (isCellValid(adjacentCell)) {
                    cells.push_back(adjacentCell);
                }
            }
            return cells;
        }
        
        void performActionOnCell(const std::function<void(size_t, size_t, CellType&)> action) {
            for (auto rowIndex = 0; rowIndex < rows; rowIndex++) {
                for (auto columnIndex = 0; columnIndex < columns; columnIndex++) {
                    action(rowIndex, columnIndex, (*this)[rowIndex][columnIndex]);
                }
            }
        }
        
        bool operator==(const GameBoard & gameBoard) const {
            if (rows != gameBoard.rows || columns != gameBoard.columns) {
                return false;
            }
            for (auto i = 0; i < rows; i++) {
                for (auto j = 0; j < columns; j++) {
                    if ((*this)[i][j] != gameBoard[i][j]) {
                        return false;
                    }
                }
            }
            return true;
        }
    };
}

namespace std {
    
    template <>
    struct hash<GameBoard::CellPosition>
    {
        std::size_t operator()(const GameBoard::CellPosition& position) const
        {
            using std::size_t;
            using std::hash;
            using std::string;
            auto seed = position.row;
            seed ^= position.column + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}

#endif /* GameBoard_hpp */
