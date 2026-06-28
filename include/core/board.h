// core/board.h
// a file for board stuff
// TODO: Future refactor - Bundle ChessBoardMatrix, TurnColor, and
// CastlingRights into a unified Board struct once core features are fully
// stable.
#pragma once

#include <raylib.h>
#include <string>

#include "core/logic.h"
#include "pieces/pieces.h"
// #include "types.h" // Include your TurnColor / ChessBoardMatrix definitions
// if they are elsewhere

#ifndef CHESSY_BOARD_H
#define CHESSY_BOARD_H

namespace ChessCore::logic {
using TurnColor = PieceColor;

struct Board {
  // 1. The Core State Variables
  ChessBoardMatrix matrix; // The actual 8x8 piece grid
  TurnColor currentTurn = TurnColor::WHITE_PIECE;
  ChessLogic::CastlingRights castlingRights;

  // Optional expansions later:
  // int enPassantCol = -1;              // Track active en passant columns
  // std::vector<Move> moveHistory;       // For undo/redo features

  // 2. Fundamental State Mutators (Keep 'em clean and self-contained)
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

} // namespace ChessCore::logic

namespace ChessCore::ui {

// here, since we have defined board above and don't want to conflict myself...
// I am gonna use C-styled function-struct, no outside struct stuff.

// struct containing these modes.
// The modes here are for... say, if the board is used for
// - practice -> RESET
// - puzzles
// - analysis -> separate instantiation to not conflict with the two.
// - online -> separate instantiation to not conflict with the two.
//             also will reset if one leaves it
// - computer -> loading from file + separate instantiation (if file is not
// present)
// ---
// currentnotes.=page
// NOTE:
//
// - the 4th option, `online` is not gonna be used
// the same goes for the 3rd and the 5th.
// - analysis is a nice future implementation that will have have redo/undo
// same goes for practice.
struct ChessBoardMode {

  // not set to private to encourage modification
  bool practice;
  bool puzzles;
  bool analysis;
  bool online;
  bool computer;

  // You don't wanna mistype stuff do you
  std::string mod_practice = "practice";
  std::string mod_puzzles = "puzzle";
  std::string mod_analysis = "analyze";
  std::string mod_online = "online";
  std::string mod_computer = "computer";

  void SetMode(std::string mode_name) {
    if (mode_name == "practice")
      practice = true;
    if (mode_name == "puzzle")
      puzzles = true;
    if (mode_name == "analyze")
      analysis = true;
    if (mode_name == "online")
      online = true;
    if (mode_name == "computer")
      computer = true;
  }

  // @brief Check if there are bugs.
  // @returns `bool`
  // - If there are no bugs, `false`
  // - If there are... `true`
  bool CheckBugs() {
    // basically.. make sure that we have stuff
    // running. since i didn't use an enum, guess like you and
    // me have to suffer pain

    // NOTE: put the `if (...) { ... return true; }` logic here
    if (currentMode == mod_practice)
      if (puzzles || online)
        return true;
    if (currentMode == mod_online)
      if (practice || puzzles || analysis || computer)
        return true;
    // ...

    return false; // ELSE... we are free of bugs...
  }

private:
  std::string_view currentMode;
};

// function for checking if we have been in *what* mode

} // namespace ChessCore::ui

#endif
