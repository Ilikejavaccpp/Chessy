#pragma once

#include <cmath>

#include "core/logic.h"
#include "pieces/pieces.h"

#ifndef CHESSY_PIECE_KING_H
#define CHESSY_PIECE_KING_H

namespace ChessLogic {
struct CastlingRights;
}

namespace ChessLogic {
// inline bool CanCastle(PieceColor color, bool kingSide, ChessBoardMatrix
// &board,
//                       CastlingRights &rights) {
//   // HACK: Forward declaration— to be implemented in `logic.h`
//   return false;
// }

inline bool CanCastle(PieceColor color, bool kingSide, ChessBoardMatrix &board,
                      CastlingRights &rights);
} // namespace ChessLogic

namespace ChessLogic {
inline bool IsKingMoveLegal(int fRow, int fCol, int tRow, int tCol,
                            PieceColor turn, ChessBoardMatrix &board,
                            CastlingRights &rights) {
  int rowDist = std::abs(tRow - fRow);
  int colDist = std::abs(tCol - fCol);

  // Block the move if the King tries to step more than 1 square away
  // With exeption of castling -> TODO_DONE
  // return (rowDist <= 1 && colDist <= 1) || CanCastle(turn, true, board,
  // rights);

  // 1. Intercept Castling attempts: King must stay on same row, and jump
  // exactly 2 columns
  int homeRow = (turn == WHITE_PIECE) ? 7 : 0;

  // If on 5th rank (col 4th index or col no. 5), then check if it queenside or
  // kingside
  if (fRow == homeRow && tRow == homeRow && fCol == 4 && colDist == 2) {
    // If destination column is 6 (g1/g8), it's a Kingside attempt
    if (tCol == 6) {
      return CanCastle(turn, true, board, rights);
    }
    // If destination column is 2 (c1/c8), it's a Queenside attempt
    if (tCol == 2) {
      return CanCastle(turn, false, board, rights);
    }
  }

  // 2. Check for standard 1-square steps in any direction
  if (rowDist <= 1 && colDist <= 1) {
    return true;
  }

  // If it's not a standard step or a validated castling attempt, it's illegal
  return false;
}
} // namespace ChessLogic

#endif
