#pragma once

#include <cmath>

#ifndef CHESSY_PIECE_KNIGHT_H
#define CHESSY_PIECE_KNIGHT_H

namespace ChessLogic {
inline bool IsKnightMoveLegal(int fRow, int fCol, int tRow, int tCol) {
  int rowDist = std::abs(tRow - fRow);
  int colDist = std::abs(tCol - fCol);

  // Exact mathematical confirmation of an L-shape step
  return (rowDist == 2 && colDist == 1) || (rowDist == 1 && colDist == 2);
}

// TODO: implement this
inline bool IsKnightDraggable() { return false; }

} // namespace ChessLogic

#endif
