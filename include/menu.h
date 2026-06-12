#pragma once

// COLORSCHEME
// #define CHESSY_COLROSCHEME_ARC_SOFT

#include <raylib.h>

#include "core/colorscheme.h"
#include "core/logic.h"
#include "pieces/pieces.h"
#include "utils.h"

// The turn of a player, returns black (color) or (white)
// This is an alias of `piece/pieces.h/@ PieceColor` to not get confused
using TurnColor = PieceColor; // to not get confused when defining turns

// Just for habits
namespace ChessUI {
// CHESSY_UI_MENU_MODE
// was defined here. MAY PUT OTHER STUFF THO
}

namespace ChessUI {
inline void DrawSelectedPieceUIDots(ChessMouseInteraction mouseInteraction,
                                    TurnColor currentTurn,
                                    ChessBoardMatrix &boardState,
                                    ChessLogic::CastlingRights &rights,
                                    Colorscheme &palette, Sound soundCheck) {
  if (mouseInteraction.selectedRow != -1 &&
      mouseInteraction.selectedCol != -1) {
    // Compute possible destinations for the piece right before drawing
    auto possibleMoves = ChessLogic::GetLegalMovesForPiece(
        mouseInteraction.selectedRow, mouseInteraction.selectedCol, currentTurn,
        boardState, rights);

    // Draw the dot/ring overlays cleanly over the empty or occupied target
    // squares
    ChessVisuals::DrawMovePossibilityDots(palette, possibleMoves, boardState);
  }
}

// draws a red Square or `check_square` key from palette when the king is
// checked with sound (TODO)
inline void DrawBoardUIKingChecked(
    TurnColor &currentTurn, ChessBoardMatrix &boardState,
    ChessLogic::CastlingRights &rights, Colorscheme &palette, Sound soundCheck,
    bool &hasPlayedSound,
    int &currentScreenMode, // you can't pass `ChessMenu::CHESSY_UTIL_MENU_MODE`
                            // here since that will result in a compiler panic
                            // about non-const lvalue param reference. Since
                            // anyways the enum members convert to `int`, pass
                            // it like so
    ChessUI::CHESSY_UI_MENU_MODE &currentMenuMode,

    // These 2 params are for capturing cleanup. may make this into a dedicated
    // vector or array of captures, with a nice looping to sort things out. ->
    // TODO

    std::vector<PieceType> &whiteCaptured,
    std::vector<PieceType> &blackCaptured) {

  // Draw a deep red warning block under the King if checked
  if (ChessLogic::IsKingInCheck(currentTurn, boardState, rights)) {
    int kRow = -1, kCol = -1;
    if (ChessLogic::FindKingCoordinates(currentTurn, boardState, kRow, kCol)) {
      DrawRectangle(boardOffsetX + (kCol * squareSize),
                    boardOffsetY + (kRow * squareSize), squareSize, squareSize,
                    palette["check_square"]);

      if (hasPlayedSound == false) {
        PlaySound(soundCheck);
        hasPlayedSound = true;
      }

      if (ChessLogic::GetLegalMovesForPiece(kRow, kCol, currentTurn, boardState,
                                            rights)
              .empty()) {

        // Return to the home page
        if (hasPlayedSound == true) { // since this will be reset, hence once.
          std::cout << "[INFO] : LOGIC -- Checkmate, returning to HOME. -> "
                       "REDIRECT\n";
        }
        currentScreenMode = ChessMode::CHESSY_MODE_NORMAL;
        currentMenuMode = CHESSY_MODE_HOME;

        // Cleanup
        hasPlayedSound = false;
        whiteCaptured.clear();
        blackCaptured.clear();

        // Start a fresh new game
        initStartingPosition(boardState); // clear left over remnants
        currentTurn = WHITE_PIECE;

        // PlaySound(soundCheckmate);
      }
    }

  } else {
    hasPlayedSound = false;
  }
}
} // namespace ChessUI

namespace ChessMode {
enum CHESSY_UTIL_MENU_MODE : int {
  CHESSY_MODE_NORMAL_VAL = 0,
  CHESSY_MODE_PLAYCF_VAL = 1
};

// Map the extern tracking links to the true compiler values
inline constexpr CHESSY_UTIL_MENU_MODE CHESSY_MODE_NORMAL =
    CHESSY_MODE_NORMAL_VAL;
inline constexpr CHESSY_UTIL_MENU_MODE CHESSY_MODE_PLAYCF =
    CHESSY_MODE_PLAYCF_VAL;
} // namespace ChessMode

namespace ChessMenu {

struct MenuUI {
  Rectangle navAbout, navHelp, navNews, navOpeningLib, navImpExport;
  Rectangle btnPlayOnline, btnPlayComputer, btnDailyPuzzles, btnPracticeMode;
};

// ALIGNED: Takes clean layout floats instead of hiding internal Raylib window
// calls
inline MenuUI GetDynamicLayout(float w, float h) {
  MenuUI ui;

  // 1. Navigation Header Links (Matched to draw text Y: 33, size: 21)
  ui.navAbout = {35.0f, 28.0f, 65.0f, 30.0f};
  ui.navHelp = {145.0f, 28.0f, 55.0f, 30.0f};
  ui.navNews = {235.0f, 28.0f, 55.0f, 30.0f};
  ui.navOpeningLib = {325.0f, 28.0f, 175.0f, 30.0f};
  ui.navImpExport = {535.0f, 28.0f, 125.0f, 30.0f};

  // 2. Compute Container Layout Properties
  float boardSize = h - 130.0f;
  float panelX = 20.0f + boardSize + 20.0f;
  float panelW = w - panelX - 20.0f;

  // 3. Right Sidebar Interactive Buttons (Calibrated to panel bounds)
  float bX = panelX + 20.0f;
  float bW = panelW - 40.0f;
  float bH = (boardSize - 50.0f) / 4.0f; // Even division for 4 slots
  float gap = 12.0f;
  float startY = 120.0f;

  ui.btnPlayOnline = {bX, startY, bW, bH - gap};
  ui.btnPlayComputer = {bX, startY + bH, bW, bH - gap};
  ui.btnDailyPuzzles = {bX, startY + (2.0f * bH), bW, bH - gap};
  ui.btnPracticeMode = {bX, startY + (3.0f * bH), bW, bH - gap};

  return ui;
}

// TUNED: Matches drawing parameters exactly
inline void UpdateMenuInput(ChessUI::CHESSY_UI_MENU_MODE &currentMode,
                            ChessMode::CHESSY_UTIL_MENU_MODE &currentGameMode,
                            int width, int height) {
  float w = static_cast<float>(width);
  float h = static_cast<float>(height);
  MenuUI ui = GetDynamicLayout(w, h);
  Vector2 mouse = GetMousePosition();

  bool isHovering = CheckCollisionPointRec(mouse, ui.navAbout) ||
                    CheckCollisionPointRec(mouse, ui.navHelp) ||
                    CheckCollisionPointRec(mouse, ui.navNews) ||
                    CheckCollisionPointRec(mouse, ui.navOpeningLib) ||
                    CheckCollisionPointRec(mouse, ui.navImpExport) ||
                    CheckCollisionPointRec(mouse, ui.btnPlayOnline) ||
                    CheckCollisionPointRec(mouse, ui.btnPlayComputer) ||
                    CheckCollisionPointRec(mouse, ui.btnDailyPuzzles) ||
                    CheckCollisionPointRec(mouse, ui.btnPracticeMode);

  SetMouseCursor(isHovering ? MOUSE_CURSOR_POINTING_HAND
                            : MOUSE_CURSOR_DEFAULT);

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    if (CheckCollisionPointRec(mouse, ui.navAbout))
      currentMode = ChessUI::CHESSY_MODE_ABOUT;
    if (CheckCollisionPointRec(mouse, ui.navHelp))
      currentMode = ChessUI::CHESSY_MODE_HELP;
    if (CheckCollisionPointRec(mouse, ui.navNews))
      currentMode = ChessUI::CHESSY_MODE_NEWS;
    if (CheckCollisionPointRec(mouse, ui.navOpeningLib))
      currentMode = ChessUI::CHESSY_MODE_OPENING;
    if (CheckCollisionPointRec(mouse, ui.navImpExport))
      currentMode = ChessUI::CHESSY_MODE_IMPEXP;

    if (CheckCollisionPointRec(mouse, ui.btnPlayOnline)) {
      currentMode = ChessUI::CHESSY_MODE_ONLINE;
      currentGameMode = ChessMode::CHESSY_MODE_PLAYCF;
      SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
    if (CheckCollisionPointRec(mouse, ui.btnPlayComputer)) {
      currentMode = ChessUI::CHESSY_MODE_COMPUTER;
      currentGameMode = ChessMode::CHESSY_MODE_PLAYCF;
      SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
    if (CheckCollisionPointRec(mouse, ui.btnPracticeMode)) {
      currentMode = ChessUI::CHESSY_MODE_PRACTICE;
      currentGameMode = ChessMode::CHESSY_MODE_PLAYCF;
      SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
  }
}

inline void DrawStartMenu(Colorscheme &palette, Font &font, int width,
                          int height, Shader &sdfShader) {
  ClearBackground(palette["background_dark"]);

  float w = static_cast<float>(width);
  float h = static_cast<float>(height);

  // ALIGNED: Feeds identical frame floats directly into the drawer engine
  // layout pass
  MenuUI ui = GetDynamicLayout(w, h);
  Vector2 mouse = GetMousePosition();

  // 1. Navigation Header Layout
  DrawRectangle(20, 20, w - 40, 50, palette["background_dark_menu_header"]);

  auto GetLinkColor = [&](Rectangle rec) {
    return CheckCollisionPointRec(mouse, rec) ? palette["accent"]
                                              : palette["foreground_dark"];
  };

  // BeginShaderMode(sdfShader);

  DrawTextEx(font, "About", {35, 33}, 21, 1.4f, GetLinkColor(ui.navAbout));
  DrawTextEx(font, "Help", {145, 33}, 21, 1.4f, GetLinkColor(ui.navHelp));
  DrawTextEx(font, "News", {235, 33}, 21, 1.4f, GetLinkColor(ui.navNews));
  DrawTextEx(font, "Opening Library", {325, 33}, 21, 1.4f,
             GetLinkColor(ui.navOpeningLib));
  DrawTextEx(font, "Imp/Export", {535, 33}, 21, 1.4f,
             GetLinkColor(ui.navImpExport));

  // 2. Left Side Menu Canvas Board Container
  float boardSize = h - 130.0f;
  DrawRectangle(20, 90, boardSize, boardSize,
                palette["background_dark_menu_body"]);
  DrawTextEx(font, "[ Dynamic Menu Board ]",
             {20 + (boardSize / 4.0f), 90 + (boardSize / 2.0f) - 10}, 18, 1.0f,
             palette["foreground_dark"]);
  DrawRectangleLinesEx({20, 90, boardSize, boardSize}, 1,
                       palette["background_dark_menu_header"]);

  // 3. Right Side Menu Sidebar Box Container
  float panelX = 20 + boardSize + 20;
  float panelW = w - panelX - 20;
  DrawRectangle(panelX, 90, panelW, boardSize,
                palette["background_dark_menu_header"]);

  // Custom button drawer
  // Lambda function so that we don't have a gazillion functions.
  // There are too many functions in this project
  auto DrawMenuButton = [&](Rectangle rec, const char *label) {
    bool hover = CheckCollisionPointRec(mouse, rec);
    DrawRectangleRec(rec, hover ? palette["hover_button"]
                                : palette["background_dark_menu_body"]);
    DrawRectangleLinesEx(rec, 1,
                         hover ? palette["hover_button_outline"]
                               : palette["background_dark_menu_header"]);
    DrawTextEx(
        font, label, {rec.x + 20, rec.y + (rec.height / 2.0f) - 10}, 20, 1.2f,
        hover ? palette["hover_button_text"] : palette["foreground_dark"]);
  };

  DrawMenuButton(ui.btnPlayOnline, "Play Online / Friend");
  DrawMenuButton(ui.btnPlayComputer, "Play vs Computer");
  DrawMenuButton(ui.btnDailyPuzzles, "Daily Puzzles");
  DrawMenuButton(ui.btnPracticeMode, "Self Practice Mode");

  // EndShaderMode();

  DrawRectangleLinesEx({panelX, 90, panelW, boardSize}, 1,
                       palette["background_dark_menu_body"]);
}

inline void DrawAboutMenu(Colorscheme &palette, Font &font, int width,
                          int height, Shader &sdfShader) {
  ClearBackground(palette["background_dark"]);

  float w = static_cast<float>(width);
  float h = static_cast<float>(height);

  // ALIGNED: Feeds identical frame floats directly into the drawer engine
  // layout pass
  MenuUI ui = GetDynamicLayout(w, h);
  Vector2 mouse = GetMousePosition();

  // The button for home. must be so that once we click about again, it
  // teleports us to home.
  // Therefore, let's use the home header row, again.
  DrawRectangle(20, 20, w - 40, 50, palette["background_dark_menu_header"]);

  auto GetLinkColor = [&](Rectangle rec) {
    return CheckCollisionPointRec(mouse, rec) ? palette["accent"]
                                              : palette["foreground_dark"];
  };

  // BeginShaderMode(sdfShader); // for some reason, this really fucks up the
  //                             // font.
  //                             // and I don't want the scenario where it
  //                             // mysteriously stops working.

  // The headers
  DrawTextEx(font, "About", {35, 33}, 21, 1.4f, GetLinkColor(ui.navAbout));
  DrawTextEx(font, "Help", {145, 33}, 21, 1.4f, GetLinkColor(ui.navHelp));
  DrawTextEx(font, "News", {235, 33}, 21, 1.4f, GetLinkColor(ui.navNews));
  DrawTextEx(font, "Opening Library", {325, 33}, 21, 1.4f,
             GetLinkColor(ui.navOpeningLib));
  DrawTextEx(font, "Imp/Export", {535, 33}, 21, 1.4f,
             GetLinkColor(ui.navImpExport));

  // Custom button drawer
  // Lambda function so that we don't have a gazillion functions.
  // There are too many functions in this project
  auto DrawMenuButton = [&](Rectangle rec, const char *label) {
    bool hover = CheckCollisionPointRec(mouse, rec);
    DrawRectangleRec(rec, hover ? palette["hover_button"]
                                : palette["background_dark_menu_body"]);
    DrawRectangleLinesEx(rec, 1,
                         hover ? palette["hover_button_outline"]
                               : palette["background_dark_menu_header"]);
    DrawTextEx(
        font, label, {rec.x + 20, rec.y + (rec.height / 2.0f) - 10}, 20, 1.2f,
        hover ? palette["hover_button_text"] : palette["foreground_dark"]);
  };

  // Check the collision points and update in the next big function `utils.h`.
  // The reviewer will be cooked i guess... Welp i am the reviewer so :sob:
  // :sob:
}
} // namespace ChessMenu
