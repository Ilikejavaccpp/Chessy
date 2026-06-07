
#pragma once

// #smartthinking
// Since the queen is basically both a rook and a bishop
#include "bishop.h"
#include "rook.h"

#ifndef CHESSY_PIECE_QUEEN_H
#define CHESSY_PIECE_QUEEN_H

namespace ChessLogic {
inline bool IsQueenMoveLegal(int fRow, int fCol, int tRow, int tCol,
                             const ChessBoardMatrix &board) {
  // Reuse the clean Rook and Bishop validation functions directly
  return IsRookMoveLegal(fRow, fCol, tRow, tCol, board) ||
         IsBishopMoveLegal(fRow, fCol, tRow, tCol, board);
}
} // namespace ChessLogic

#endif
