// Provides the base for the pieces
// The logic will be implemented later via their own separate files.
#pragma once

#ifndef CHESSY_PIECES_H
#define CHESSY_PIECES_H

enum PieceType { EMPTY = 0, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum PieceColor { NONE_COLOR = 0, WHITE_PIECE, BLACK_PIECE };

struct ChessPiece {
  PieceType type = EMPTY;
  PieceColor color = NONE_COLOR;
};

using ChessBoardMatrix = ChessPiece[8][8];

namespace ChessGlyphs {
// DEPECRATED
// // White Pieces Unicode mappings
// inline const char *W_PAWN = "\u2659";
// inline const char *W_KNIGHT = "\u2658";
// inline const char *W_BISHOP = "\u2657";
// inline const char *W_ROOK = "\u2656";
// inline const char *W_QUEEN = "\u2655";
// inline const char *W_KING = "\u2654";
//
// // Black Pieces Unicode mappings
// inline const char *B_PAWN = "\u265F";
// inline const char *B_KNIGHT = "\u265E";
// inline const char *B_BISHOP = "\u265D";
// inline const char *B_ROOK = "\u265C";
// inline const char *B_QUEEN = "\u265B";
// inline const char *B_KING = "\u265A";

inline const char *B_PAWN = u8"♙";   // ♙
inline const char *B_KNIGHT = u8"♘"; // ♘
inline const char *B_BISHOP = u8"♗"; // ♗
inline const char *B_ROOK = u8"♖";   // ♖
inline const char *B_QUEEN = u8"♕";  // ♕
inline const char *B_KING = u8"♔";   // ♔

inline const char *W_PAWN = u8"♟";   // ♟
inline const char *W_KNIGHT = u8"♞"; // ♞
inline const char *W_BISHOP = u8"♝"; // ♝
inline const char *W_ROOK = u8"♜";   // ♜
inline const char *W_QUEEN = u8"♛";  // ♛
inline const char *W_KING = u8"♚";   // ♚
} // namespace ChessGlyphs

#endif
