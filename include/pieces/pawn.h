#pragma once

#include "pieces.h"
#include <cmath>

#ifndef CHESSY_PIECE_PAWN_H
#define CHESSY_PIECE_PAWN_H

namespace ChessLogic {
inline bool IsPawnMoveLegal(int fRow, int fCol, int tRow, int tCol,
                            PieceColor color, const ChessBoardMatrix &board) {
  // White moves up (-1 row), Black moves down (+1 row)
  int direction = (color == WHITE_PIECE) ? -1 : 1; // Up or Down
  int startRow = (color == WHITE_PIECE) ? 6 : 1;   // 1st row or 7th row

  int rowDist = tRow - fRow;           // calculate the row distance via new-old
  int colDist = std::abs(tCol - fCol); // round the displacement into distance

  // Case A: Single Step Forward into empty space
  if (fCol == tCol && rowDist == direction && board[tRow][tCol].type == EMPTY) {
    return true;
  }

  // Case B: Initial Double Step Forward from starting rank
  if (fCol == tCol && fRow == startRow && rowDist == (2 * direction)) {
    // Verify BOTH the intermediate square and target square are empty
    if (board[fRow + direction][fCol].type == EMPTY &&
        board[tRow][tCol].type == EMPTY) {
      return true;
    }
  }

  // Case C: Standard Diagonal Capture
  if (colDist == 1 && rowDist == direction && board[tRow][tCol].type != EMPTY) {
    return true;
  }

  // TODO: implement en-passant

  return false; // If no conditions are met, the move is illegal
}
} // namespace ChessLogic

#endif
