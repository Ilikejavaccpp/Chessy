#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <raylib.h>
#include <string>

#include "core/colorscheme.h"
#include "core/dimensions.h"
#include "core/logic.h"
#include "menu.h"
#include "pieces/pieces.h"
#include "utils.h"

#ifndef CHESSY_GAME_H
#define CHESSY_GAME_H

// ChessApplication
class ChessApplication {
private:
  /**********************************************************
   * Settings and uninitialized variables
   ***********************************************************/
  int width, height;
  std::string title;
  bool initialized;

  Colorscheme palette;

  // 0 -> pawn
  // 1 -> knight (N)
  // 2 -> bishop (B)
  // 3 -> rook (R)
  // 4 -> queen (Q)
  // 5 -> king (K)
  Texture2D whiteTextures[6];
  Texture2D blackTextures[6];
  std::vector<PieceType> whiteCaptured;
  std::vector<PieceType> blackCaptured;

  std::string font_path = "assets/fonts/nffont.ttf";
  Font font;

  ChessBoardMatrix boardState; // Allocates 64 contiguous memory blocks
  bool pieceSelected = false;
  int selectedRow = -1;
  int selectedCol = -1;

  // Private members variables (for turn based and ruled) block:
  PieceColor currentTurn = WHITE_PIECE;
  ChessMouseInteraction mouseInteraction;
  ChessLogic::CastlingRights castling_rights;

  /**********************************************************
   * Helper functions that are necessary.
   **********************************************************/

  // REDUNDANT:
  // void update() {
  //   if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
  //     Vector2 mousePos = GetMousePosition();
  //
  //     // Translate raw mouse pixel vectors directly into 0-7 array indices
  //     int col = (mousePos.x - boardOffsetX) / squareSize;
  //     int row = (mousePos.y - boardOffsetY) / squareSize;
  //
  //     // Ensure the click landed inside our 8x8 chessboard perimeter
  //     if (col >= 0 && col < boardSize && row >= 0 && row < boardSize) {
  //
  //       if (!pieceSelected) {
  //         // STEP 1: Attempt to pick up a piece
  //         if (boardState[row][col].type != EMPTY) {
  //           pieceSelected = true;
  //           selectedRow = row;
  //           selectedCol = col;
  //           std::cout << "[INPUT] : Piece Selected at Rank " << row << ",
  //           File "
  //                     << col << "\n";
  //         }
  //       } else {
  //         // STEP 2: An item is already selected. Attempt to execute a move
  //         // target
  //         if (row == selectedRow && col == selectedCol) {
  //           // If clicking the exact same square, simply deselect the piece
  //           pieceSelected = false;
  //           selectedRow = -1;
  //           selectedCol = -1;
  //           std::cout << "[INPUT] : Selection Cancelled.\n";
  //         } else {
  //           // Basic core move logic: Transfer state data over to the new
  //           // position block
  //           boardState[row][col] = boardState[selectedRow][selectedCol];
  //
  //           // Wipe old location clean back to empty spaces
  //           boardState[selectedRow][selectedCol] = {EMPTY, NONE_COLOR};
  //
  //           // Reset tracking variables to prepare for the next turn loop
  //           pieceSelected = false;
  //           selectedRow = -1;
  //           selectedCol = -1;
  //           std::cout
  //               << "[MOVE] : Relocated piece cleanly to destination
  //               square.\n";
  //         }
  //       }
  //     }
  //   }
  // }
  //
  // // The draw function, needs dependancy file (chessy header file) `utils.h`
  // void draw(Font &font) {
  //   BeginDrawing();
  //
  //   ClearBackground(palette["background_dark"]);
  //   ChessVisuals::DrawChessboard(palette);
  //   ChessVisuals::DrawSelectedHighlight(palette, pieceSelected, selectedRow,
  //                                       selectedCol);
  //   ChessVisuals::DrawHoverHighlight(palette);
  //   ChessVisuals::DrawPiecesWithTextures(boardState, whiteTextures,
  //                                        blackTextures);
  //   ChessVisuals::DrawGameUI(palette, font, whiteCaptured, blackCaptured);
  //
  //   EndDrawing();
  // }
  /*
    BeginDrawing();

    // Use a nice background
    ClearBackground(palette["background_dark"]);

    // Draw the chessboard
    ChessVisuals::DrawChessboard(palette);

    // Update highlights to track the unified click status
    ChessVisuals::DrawActiveHighlights(palette, mouseInteraction);
    ChessVisuals::DrawHoverHighlight(palette);

    ChessUI::DrawSelectedPieceUIDots(mouseInteraction, currentTurn, boardState,
                                     castling_rights, palette);

    // Draw a deep red warning block under the King if checked
    ChessUI::DrawBoardUIKingChecked(currentTurn, boardState, castling_rights,
                                    palette);

    // Paint piece textures with hybrid mouse tracking
    ChessVisuals::DrawPiecesWithHybridControls(boardState, whiteTextures,
                                               blackTextures, mouseInteraction);

    // Draw the UI (Menu)
    ChessVisuals::DrawGameUI(palette, font, whiteCaptured, blackCaptured,
                             whiteTextures, blackTextures);

    EndDrawing();
  */

  // The update function, needs dependancy file (chessy header file) `utils.h`
  void update() {
    // Fullscreen default
    if (IsKeyPressed(KEY_F11)) {
      ToggleFullscreen();
    }

    // Update if resized (no dependancy from project)
    // except the dimensions
    if (IsWindowResized() || IsKeyPressed(KEY_F11)) {
      this->width = GetScreenWidth();
      this->height = GetScreenHeight();

      // 3. Recalculate your global dimensions layout math live!
      // This ensures the board and panels scale cleanly with the resolution.
      squareSize = (this->height - 130) / 8;
      boardOffsetY = 90;
      boardOffsetX = 20;

      // Dynamic side panel width update
      panelWidth = this->width - (boardOffsetX + (8 * squareSize)) - 40;
    }

    // Pass state wrappers directly down to the hybrid input managers
    ChessInput::ProcessDualInput(boardState, currentTurn, mouseInteraction,
                                 whiteCaptured, blackCaptured, castling_rights);
  }

  void draw() { ChessMenu::DrawStartMenu(palette, font); }

  // This function is used by the destructor
  // NOTE: Please don't call it in your main function.
  //       I am just too lazy to make it a private function
  void close() { CloseWindow(); }

public:
  ChessApplication(int width, int height, const char *title) {
    this->width = width;
    this->height = height;
    this->title = std::string(title);
    this->initialized = true;

    this->palette.init();

    initStartingPosition(boardState);
  }
  // NOTE: destructor may be implemented lator on
  ~ChessApplication() {
    if (this->initialized) {
      std::cout << "[INFO] : Closing all instances of raylib and other "
                   "initialized services...\n";

      std::cout << "[INFO] : Unloading the board state... -> WAIT\n";
      currentTurn = PieceColor::NONE_COLOR;
      std::cout
          << "[INFO] : Successfully unloaded the board state... -> SUCCESS\n";

      // NOTE: will put future deinitialization / unloading of textures, etc.
      std::cout << "[INFO] :  Unloading textures... -> WAIT\n";
      for (uint8_t i = 0; i < 6; i++) {
        UnloadTexture(whiteTextures[i]);
        std::cout << "[INFO] : Unloaded white piece texture, -> NAME "
                     "whiteTexture.id="
                  << int(i) << "\n";
        UnloadTexture(blackTextures[i]);
        std::cout << "[INFO] : Unloaded black piece texture, -> NAME "
                     "blackTexture.id="
                  << int(i) << "\n";
      }
      std::cout << "[INFO] : Successfully unloaded all textures -> SUCCESS\n";

      std::cout << "[INFO] : Closing Raylib and unloading raylib instances... "
                   "-> WAIT\n";
      std::cout << "[INFO] : Successfully unloaded font with path " << font_path
                << ".-> SUCCESS\n";

      // Closes the Window
      close();

      std::cout << "[INFO] : Successfully closed raylib. -> SUCCESS\n";
      std::cout << "[INFO] : Exiting with [code 0]...\n";
    }
  }

  /**********************************************************
   * Core functions/methods that are neccessary
   **********************************************************/

  // Starts the application and the game loop
  void run() {
    if (this->title.empty() || (this->height < 100) || (this->width < 100)) {
      std::cout << "[ERROR] : Invalid initialization in chessy's "
                   "`include/core/game.h`. Please debug or change it.\n";
      std::cout << "[INFO] : Exiting with [code 1]...";
      std::exit(1);
    }

    InitWindow(this->width, this->height, this->title.c_str());
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    SetTargetFPS(60); // may use a variable the user can change or a scroll
                      // bar that is dynamic.
    loadPieceTextures(whiteTextures, blackTextures);

    // DEPECRATED, FIXME
    // this->font = LoadFont("assets/fonts/nffont.ttf");

    this->font = ChessMenu::loadPieceGlyphs(font_path);

    mainloop();
  }

  void mainloop() {
    while (!WindowShouldClose()) {
      /**********************************************************
       * Update Block / Section
       **********************************************************/
      update();

      /**********************************************************
       * Drawing / Rendering Block / Section
       **********************************************************/
      draw();
    }
  }
};

#endif
