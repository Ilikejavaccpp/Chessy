// core/board.h
// a file for board stuff
// TODO: Future refactor - Bundle ChessBoardMatrix, TurnColor, and
// CastlingRights into a unified Board struct once core features are fully
// stable.
#pragma once

#include <raylib.h>

#include "core/logic.h"
#include "pieces/pieces.h"
// #include "types.h" // Include your TurnColor / ChessBoardMatrix definitions
// if they are elsewhere

#ifndef CHESSY_BOARD_H
#define CHESSY_BOARD_H

namespace ChessCore {
using TurnColor = PieceColor;

struct Board {
  // 1. The Core State Variables
  ChessBoardMatrix matrix; // The actual 8x8 piece grid
  TurnColor currentTurn = TurnColor::WHITE_PIECE;
  ChessLogic::CastlingRights castlingRights;

  // Optional expansions later:
  // int enPassantCol = -1;              // Track active en passant columns
  // std::vector<Move> moveHistory;       // For undo/redo features

  // 2. Fundamental State Mutators (Keep them clean and self-contained)
  void Reset() {
    // Initialize pieces to starting layout matrix...
    currentTurn = TurnColor::WHITE_PIECE;
    // reset castling rights flags...
  }

  void SwitchTurn() {
    currentTurn = (currentTurn == TurnColor::WHITE_PIECE)
                      ? TurnColor::BLACK_PIECE
                      : TurnColor::WHITE_PIECE;
  }
};

} // namespace ChessCore

#endif
