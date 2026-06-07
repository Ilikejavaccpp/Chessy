#pragma once

#include "pieces.h"
#include <cmath>

#ifndef CHESSY_PIECE_BISHOP_H
#define CHESSY_PIECE_BISHOP_H

namespace ChessLogic {
inline bool IsBishopMoveLegal(int fRow, int fCol, int tRow, int tCol,
                              const ChessBoardMatrix &board) {
  int rowDist = std::abs(tRow - fRow);
  int colDist = std::abs(tCol - fCol);

  if (rowDist != colDist)
    return false; // Must be perfectly diagonal

  // Establish directional steps (+1 or -1)
  int stepRow = (tRow > fRow) ? 1 : -1;
  int stepCol = (tCol > fCol) ? 1 : -1;

  int currRow = fRow + stepRow;
  int currCol = fCol + stepCol;

  // Slide forward until we hit the destination square
  while (currRow != tRow && currCol != tCol) {
    if (board[currRow][currCol].type != EMPTY)
      return false; // Path blocked!
    currRow += stepRow;
    currCol += stepCol;
  }
  return true;
}
} // namespace ChessLogic

#endif
