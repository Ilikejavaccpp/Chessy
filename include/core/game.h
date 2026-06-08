// file for the chess application, and mainloop.
// You can tinker with this. pls just like add the functions to `utlis.h`
// but for the sake of everyone and the devs: DON'T DELETE core functions

#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <raylib.h>
#include <string>
#include <vector>

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

  // Audio Bank
  Sound soundMove;
  Sound soundCapture;
  Sound soundCheck;
  Sound soundCastle;

  // Colorscheme palette, if no colorscheme then it defaults to chessy_dark.
  // Always defaults to chessy_dark unless you override it with your own
  // colorscheme or `init()` implementation
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
  Shader sdfShader;

  std::string font_path = "assets/fonts/nffont.ttf";
  Font font;
  Font font_normal;

  ChessBoardMatrix boardState; // Allocates 64 contiguous memory blocks
  ChessMenu::MenuUI start_ui;

  // Private members variables (for turn based and ruled) block:
  PieceColor currentTurn = WHITE_PIECE;
  ChessMouseInteraction mouseInteraction;
  ChessLogic::CastlingRights castling_rights;

  // Modern Screen Mode Manager: 0 = Start Menu, 1 = Self Practice Mode
  int currentScreenMode = 0;

  bool hasChecked = false;

  /**********************************************************
   * Helper functions that are necessary.
   **********************************************************/

  // The update function, needs dependency file (chessy header file) `utils.h`
  void update() {
    // 1. Intercept native desktop fullscreen shortcut toggle commands
    if (IsKeyPressed(KEY_F11)) {
      ToggleFullscreen();
    }

    // 2. Capture hardware-level resizes or sudden fullscreen resolution updates
    if (IsWindowResized() || IsKeyPressed(KEY_F11)) {
      this->width = GetScreenWidth();
      this->height = GetScreenHeight();

      // Recalculate your global layout constraints live across frame
      // transitions
      // NOTE: MAGIC NUMBER was 140
      squareSize = (this->height - 160) / 8;
      if (squareSize < 20)
        squareSize = 20; // Lower floor boundary logic defense
      boardOffsetY = 90;
      boardOffsetX = 40;

      // Scale your side panel grid cleanly alongside the remaining window
      // workspace space
      panelWidth = this->width - (boardOffsetX + (8 * squareSize)) - 60;
      if (panelWidth < 150)
        panelWidth = 150;

      ClearWindowState(FLAG_WINDOW_UNDECORATED);
    }

    // 3. Evaluate active state mechanics based on current active routing mode
    if (currentScreenMode == ChessMode::CHESSY_MODE_NORMAL) {
      // Evaluate Mouse selection inputs on the Start Menu layout grid
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse =
            GetMousePosition(); // RESTORED: Fixed undefined variable error

        // Mirror the layout metrics utilized inside the DrawStartMenu rendering
        // canvas
        int boardSize = this->height - 130;
        int panelX = 20 + boardSize + 20;
        int panelW = this->width - panelX - 20;
        int buttonX = panelX + 20;
        int buttonW = panelW - 40;
        int buttonH = (boardSize - (15 * 5)) / 4;

        // Compute the explicit bounding frame for Button 4 (Self Practice Card)
        int b4Y = (90 + 15) + (buttonH + 15) * 3;
        Rectangle b4Bounds = {(float)buttonX, (float)b4Y, (float)buttonW,
                              (float)buttonH};

        if (CheckCollisionPointRec(mouse, b4Bounds)) {
          std::cout << "[ROUTE] : User triggered Menu Card 4 -> Transition to "
                       "Self Practice Mode\n";
          currentScreenMode = 1; // Slide application into gameplay loop
        }
      }
    } else if (currentScreenMode == ChessMode::CHESSY_MODE_PLAYCF) {
      // FIXED: Swapped sound arguments order to perfectly match utils.h
      // template definition
      ChessInput::ProcessDualInput(boardState, currentTurn, mouseInteraction,
                                   whiteCaptured, blackCaptured,
                                   castling_rights, soundCapture, soundCastle,
                                   soundCheck, soundMove);
    }
  }

  void draw() {
    BeginDrawing();

    if (currentScreenMode == ChessMode::CHESSY_MODE_NORMAL) {
      // Pass the persistent, scaling width/height parameters down directly
      ChessMenu::DrawStartMenu(palette, font, this->width, this->height,
                               this->sdfShader);
    } else if (currentScreenMode == ChessMode::CHESSY_MODE_PLAYCF) {
      if (IsKeyPressed(KEY_SPACE)) {
        std::cout << "[DEBUG] : Playing sound check.\n";
        PlaySound(soundCheck);
      }
      ClearBackground(palette["background_dark"]);

      // Draw the chessboard
      ChessVisuals::DrawChessboard(palette);

      // Update highlights to track the unified click status
      ChessVisuals::DrawActiveHighlights(palette, mouseInteraction);
      ChessVisuals::DrawHoverHighlight(palette);

      ChessUI::DrawSelectedPieceUIDots(mouseInteraction, currentTurn,
                                       boardState, castling_rights, palette,
                                       soundCheck);

      // Draw a deep red warning block under the King if checked
      ChessUI::DrawBoardUIKingChecked(currentTurn, boardState, castling_rights,
                                      palette, soundCheck, hasChecked);

      // Paint piece textures with hybrid mouse tracking
      ChessVisuals::DrawPiecesWithHybridControls(
          boardState, whiteTextures, blackTextures, mouseInteraction);

      // BeginShaderMode(this->sdfShader);
      // Draw the UI (Menu Sidebar)
      ChessVisuals::DrawGameUI(palette, font, whiteCaptured, blackCaptured,
                               whiteTextures, blackTextures);
      // EndShaderMode();
    }

    EndDrawing();
  }

  void close() { CloseWindow(); }

public:
  ChessApplication(int width, int height, const char *title) {
    this->width = width;
    this->height = height;
    this->title = std::string(title);
    this->initialized = true;

    this->palette.init();

    // Initialize the board (ready to be used.)
    initStartingPosition(boardState);
  }

  ~ChessApplication() {
    if (this->initialized) {
      std::cout << "[INFO] : Closing all instances of raylib and other "
                   "initialized services...\n";
      std::cout << "[INFO] : Unloading the board state... -> WAIT\n";
      currentTurn = PieceColor::NONE_COLOR;
      std::cout
          << "[INFO] : Successfully unloaded the board state... -> SUCCESS\n";

      std::cout << "[INFO] : Unloading sounds... -> WAIT\n";

      UnloadSound(soundCapture);
      UnloadSound(soundCastle);
      UnloadSound(soundCheck);
      UnloadSound(soundMove);

      std::cout << "[INFO] : Finished unloading sounds...\n";

      std::cout << "[INFO] : Unloading shaders... -> WAIT\n";
      UnloadShader(this->sdfShader);
      std::cout << "[INFO] : Unloaded shader -> SUCCESS\n";

      std::cout << "[INFO] :  Unloading textures... -> WAIT\n";
      for (uint8_t i = 0; i < 6; i++) {
        UnloadTexture(whiteTextures[i]);
        std::cout
            << "[INFO] : Unloaded white piece texture, -> NAME whiteTexture.id="
            << int(i) << "\n";
        UnloadTexture(blackTextures[i]);
        std::cout
            << "[INFO] : Unloaded black piece texture, -> NAME blackTexture.id="
            << int(i) << "\n";
      }
      std::cout << "[INFO] : Successfully unloaded all textures -> SUCCESS\n";

      std::cout << "[INFO] : Closing Raylib and unloading raylib instances... "
                   "-> WAIT\n";
      std::cout << "[INFO] : Successfully unloaded font with path " << font_path
                << ".-> SUCCESS\n";

      close();

      std::cout << "[INFO] : Successfully closed raylib. -> SUCCESS\n";
      std::cout << "[INFO] : Exiting with [code 0]...\n";
    }
  }

  // Starts the application and the game loop
  void run() {
    if (this->title.empty() || (this->height < 100) || (this->width < 100)) {
      std::cout << "[ERROR] : Invalid initialization in chessy's "
                   "`include/core/game.h`. Please debug or change it.\n";
      std::cout << "[INFO] : Exiting with [code 1]...";
      std::exit(1);
    }

    // Enable edge dragging mutations before instantiating window
    // WINDOWSubsystem
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(this->width, this->height, this->title.c_str());
    InitAudioDevice();

    SetTargetFPS(60);
    SetWindowMinSize(min_width,
                     min_height); // sets it so that you can't resize it down
                                  // further. FROM the wiki. Now onwards from
                                  // v1.0.3+ you will see better comments.

    BeginDrawing();
    ClearBackground(palette["background_dark"]);
    EndDrawing();

    // Load the sounds
    soundCapture = LoadSound("assets/sounds/capture.mp3");
    soundCastle = LoadSound("assets/sounds/castle.mp3");
    soundCheck = LoadSound("assets/sounds/check.mp3");
    soundMove = LoadSound("assets/sounds/move.mp3");

    // Load the textures
    loadPieceTextures(whiteTextures, blackTextures);

    // Load the shaders
    this->sdfShader = LoadShader(0, "assets/shaders/sdf.glsl");

    // Load the fonts
    this->font = loadChessFont(font_path);
    this->font_normal = loadChessFont("assets/fonts/nffont_reg.ttf");

    // Call the mainloop method.
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

#endif // CHESSY_GAME_H
