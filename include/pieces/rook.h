#pragma once

#include "pieces.h"

#ifndef CHESSY_PIECE_ROOK_H
#define CHESSY_PIECE_ROOK_H

namespace ChessLogic {
inline bool IsRookMoveLegal(int fRow, int fCol, int tRow, int tCol,
                            const ChessBoardMatrix &board) {
  // Rule 1: Must move in a straight line
  if (fRow != tRow && fCol != tCol)
    return false;

  // Rule 2: Determine step direction (-1, 0, or 1)
  int stepRow = (tRow == fRow) ? 0 : (tRow > fRow ? 1 : -1);
  int stepCol = (tCol == fCol) ? 0 : (tCol > fCol ? 1 : -1);

  int currRow = fRow + stepRow;
  int currCol = fCol + stepCol;

  // Scan the squares leading up to the destination
  while (currRow != tRow || currCol != tCol) {
    if (board[currRow][currCol].type != EMPTY)
      return false; // this means that the path is blocked
    currRow += stepRow;
    currCol += stepCol;
  }

  return true;
}
} // namespace ChessLogic

#endif
