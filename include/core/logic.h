#pragma once

#include <cstring>
// #include <iostream>
#include <vector>

#include "core/dimensions.h" // <- magic numbers HERE
#include "pieces/bishop.h"
#include "pieces/king.h"
#include "pieces/knight.h"
#include "pieces/pawn.h"
#include "pieces/pieces.h"
#include "pieces/queen.h"
#include "pieces/rook.h"
#include "raylib.h"

#ifndef CHESSY_LOGIC_H
#define CHESSY_LOGIC_H

namespace ChessLogic {

// Structure to track historical physical piece movements for castling
// authorization
struct CastlingRights {
  bool whiteKingMoved = false;
  bool whiteLeftRookMoved = false;  // Queenside (Col 0)
  bool whiteRightRookMoved = false; // Kingside (Col 7)

  bool blackKingMoved = false;
  bool blackLeftRookMoved = false;  // Queenside (Col 0)
  bool blackRightRookMoved = false; // Kingside (Col 7)
};

// Forward Declaration of primary geometric path calculator
inline bool IsMoveGeometricLegal(int fRow, int fCol, int tRow, int tCol,
                                 PieceColor currentTurn,
                                 ChessBoardMatrix &board,
                                 ChessLogic::CastlingRights &rights);

/**
 * Subroutine: Scan the board matrix to locate a color's King coordinates.
 * Needed to check vector points for king validation checks.
 */
inline bool FindKingCoordinates(PieceColor kingColor,
                                const ChessBoardMatrix &board, int &kingRow,
                                int &kingCol) {
  for (int r = 0; r < boardSize; r++) {
    for (int c = 0; c < boardSize; c++) {
      if (board[r][c].type == KING && board[r][c].color == kingColor) {
        kingRow = r;
        kingCol = c;
        return true;
      }
    }
  }
  return false;
}

/**
 * Subroutine: Check if a King is currently under attack by an enemy piece.
 * * FIXED CRITICAL RECURSION BUG:
 * Previously, evaluating if the enemy King had a geometric attack line-of-sight
 * would route through `IsKingMoveLegal`, which subsequently queried
 * `CanCastle`. That triggered an infinite recursion crash: CanCastle ->
 * IsKingInCheck -> IsKingMoveLegal -> CanCastle.
 * * To break this loop, enemy King reach checks are computed separately using
 * local coordinates. Additionally, side-effect audio triggers (`PlaySound`)
 * have been stripped out since this function runs thousands of times within
 * mock simulation passes every single frame.
 */
inline bool IsKingInCheck(PieceColor kingColor, ChessBoardMatrix &board,
                          ChessLogic::CastlingRights &rights) {
  int kRow = -1, kCol = -1;
  if (!FindKingCoordinates(kingColor, board, kRow, kCol))
    return false;

  PieceColor enemyColor =
      (kingColor == WHITE_PIECE) ? BLACK_PIECE : WHITE_PIECE;

  for (int r = 0; r < boardSize; r++) {
    for (int c = 0; c < boardSize; c++) {
      if (board[r][c].color == enemyColor) {

        // RECURSION BYPASS: Handle enemy King manually.
        // A King can attack any square immediately adjacent to it (1 square
        // distance radius).
        if (board[r][c].type == KING) {
          if (std::abs(r - kRow) <= 1 && std::abs(c - kCol) <= 1) {
            return true;
          }
          continue; // Move on to evaluating regular sliding/jumping pieces
        }

        // Evaluate regular tracking rules for all other non-king enemy pieces
        if (IsMoveGeometricLegal(r, c, kRow, kCol, enemyColor, board, rights)) {
          return true;
        }
      }
    }
  }
  return false;
}

/**
 * Subroutine: Check if a specific King side or Queen side castle action is
 * valid.
 */
inline bool CanCastle(PieceColor color, bool kingSide, ChessBoardMatrix &board,
                      CastlingRights &rights) {
  // 1. Check if the structural tracking elements or pieces moved historically
  if (color == WHITE_PIECE) {
    if (rights.whiteKingMoved)
      return false;
    if (kingSide && rights.whiteRightRookMoved)
      return false;
    if (!kingSide && rights.whiteLeftRookMoved)
      return false;
  } else {
    if (rights.blackKingMoved)
      return false;
    if (kingSide && rights.blackRightRookMoved)
      return false;
    if (!kingSide && rights.blackLeftRookMoved)
      return false;
  }

  // Set up baseline coordinate references according to team color
  int r = (color == WHITE_PIECE) ? 7 : 0;
  int kingCol = 4;
  int direction = kingSide ? 1 : -1;

  // 2. Rule: A King cannot castle *out* of an existing active check state
  if (IsKingInCheck(color, board, rights))
    return false;

  // 3. Rule: Verify path lane tiles are empty AND completely safe from active
  // enemy threats
  int checkLimit = kingSide ? 2 : 3; // Kingside tracks 2 blocks, Queenside
                                     // tracks 3 blocks for clear bounds

  for (int i = 1; i <= checkLimit; i++) {
    int nextCol = kingCol + (i * direction);

    // Path must be completely devoid of pieces
    if (board[r][nextCol].type != EMPTY)
      return false;

    // The King mechanically moves exactly 2 spaces over.
    // The squares he crosses through or lands on cannot be under attack.
    if (i <= 2) {
      // Create a virtual scratchpad array configuration to run a safe
      // simulation
      ChessBoardMatrix hypotheticalBoard;
      std::memcpy(hypotheticalBoard, board, sizeof(ChessBoardMatrix));

      // Simulate step-by-step movement positions to trap walking into check
      // fields
      hypotheticalBoard[r][nextCol] = hypotheticalBoard[r][kingCol];
      hypotheticalBoard[r][kingCol] = {EMPTY, NONE_COLOR};

      if (IsKingInCheck(color, hypotheticalBoard, rights)) {
        return false; // The move is aborted because the king would walk through
                      // or into check!
      }
    }
  }

  return true;
}

/**
 * Internal Basic Router: Evaluates spatial vector lines and raw path trajectory
 * limits.
 */
inline bool IsMoveGeometricLegal(int fRow, int fCol, int tRow, int tCol,
                                 PieceColor currentTurn,
                                 ChessBoardMatrix &board,
                                 CastlingRights &rights) {
  const ChessPiece &movingPiece = board[fRow][fCol];
  const ChessPiece &targetPiece = board[tRow][tCol];

  // Block trying to control opposition assets or self-capturing own pieces
  if (movingPiece.color != currentTurn)
    return false;
  if (targetPiece.color == currentTurn)
    return false;

  // Branch evaluation checks outwards to modular physical asset files
  switch (movingPiece.type) {
  case PAWN:
    return IsPawnMoveLegal(fRow, fCol, tRow, tCol, movingPiece.color, board);
  case ROOK:
    return IsRookMoveLegal(fRow, fCol, tRow, tCol, board);
  case KNIGHT:
    return IsKnightMoveLegal(fRow, fCol, tRow, tCol);
  case BISHOP:
    return IsBishopMoveLegal(fRow, fCol, tRow, tCol, board);
  case QUEEN:
    return IsQueenMoveLegal(fRow, fCol, tRow, tCol, board);
  case KING:
    return IsKingMoveLegal(fRow, fCol, tRow, tCol, currentTurn, board, rights);
  default:
    return false;
  }
}

/**
 * Finalized Absolute Legality Evaluator: Prevents execution of any move that
 * directly exposes or abandons your own King inside an active check field.
 */
inline bool IsMoveAbsolutelyLegal(int fRow, int fCol, int tRow, int tCol,
                                  PieceColor currentTurn,
                                  ChessBoardMatrix &board,
                                  ChessLogic::CastlingRights &rights) {
  // Step 1: Validate piece movement geometry and blocking vectors first
  if (!IsMoveGeometricLegal(fRow, fCol, tRow, tCol, currentTurn, board, rights))
    return false;

  // Step 2: Build a virtual scratchpad sandbox copy of the board to safely
  // simulate the outcome
  ChessBoardMatrix hypotheticalBoard;
  std::memcpy(hypotheticalBoard, board, sizeof(ChessBoardMatrix));

  // Shift positions inside our temporary simulated field array
  hypotheticalBoard[tRow][tCol] = hypotheticalBoard[fRow][fCol];
  hypotheticalBoard[fRow][fCol] = {EMPTY, NONE_COLOR};

  // Step 3: Void the entire move sequence if it leaves the friendly King
  // checked
  if (IsKingInCheck(currentTurn, hypotheticalBoard, rights)) {
    return false;
  }

  return true; // Safe, verified, and perfectly legal to process
}

/**
 * Loop Engine: Scans the board area matrix to fetch all coordinates a piece can
 * legally reach. Used for drawing your UI possibility overlays cleanly.
 */
inline std::vector<Vector2>
GetLegalMovesForPiece(int fRow, int fCol, PieceColor currentTurn,
                      ChessBoardMatrix &board,
                      ChessLogic::CastlingRights &rights) {
  std::vector<Vector2> valid_destinations;

  // Stop early if the target origin tile is empty or belongs to the enemy team
  if (board[fRow][fCol].type == EMPTY ||
      board[fRow][fCol].color != currentTurn) {
    return valid_destinations;
  }

  // Iterate over every coordinate position cell on the 8x8 chessboard matrix
  // array
  for (char r = 0; r < 8; ++r) {
    for (char c = 0; c < 8; ++c) {
      // Do not spend runtime calculating paths back onto the exact home square
      if (r == fRow && c == fCol)
        continue;

      // Append valid paths directly to the tracking vector stack layout
      if (IsMoveAbsolutelyLegal(fRow, fCol, r, c, currentTurn, board, rights)) {
        valid_destinations.push_back(Vector2{float(r), float(c)});
      }
    }
  }

  return valid_destinations;
}
} // namespace ChessLogic

#endif
