
#include "GameBoard.hpp"

// No idea why these functions can't be in the .hpp file :(
namespace GameBoard {
    std::ostream& operator<<(std::ostream& os, const CellPosition& cellPosition) {
        os << cellPosition.row << "," << cellPosition.column;
        return os;
    }
    
    std::ostream& operator<<(std::ostream& os, const CellSwapMove& move) {
        os << move.from.row << "," << move.from.column << " -> " << move.to.row << "," << move.to.column;
        return os;
    }
}