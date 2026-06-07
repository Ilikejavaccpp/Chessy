#pragma once

#include <raylib.h>

#include "core/colorscheme.h"
#include "utils.h"

// The turn of a player, returns black (color) or (white)
// This is an alias of `piece/pieces.h/@ PieceColor` to not get confused
using TurnColor = PieceColor; // to not get confused when defining turns

namespace ChessUI {
enum CHESSY_UI_MENU_MODE {
  CHESSY_MODE_UNSELECTED = -1,
  CHESSY_MODE_HOME = 0,
  CHESSY_MODE_ABOUT,
  CHESSY_MODE_HELP,
  CHESSY_MODE_NEWS,
  CHESSY_MODE_IMPEXP,
  CHESSY_MODE_OPENING,
  CHESSY_MODE_PRACTICE,
  CHESSY_MODE_COMPUTER,
  CHESSY_MODE_ONLINE
};
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
        boardState, rights, soundCheck);

    // Draw the dot/ring overlays cleanly over the empty or occupied target
    // squares
    ChessVisuals::DrawMovePossibilityDots(palette, possibleMoves, boardState);
  }
}

// draws a red Square or `check_square` key from palette when the king is
// checked with sound (TODO)
inline void DrawBoardUIKingChecked(TurnColor currentTurn,
                                   ChessBoardMatrix &boardState,
                                   ChessLogic::CastlingRights &rights,
                                   Colorscheme &palette, Sound soundCheck) {
  // Draw a deep red warning block under the King if checked
  if (ChessLogic::IsKingInCheck(currentTurn, boardState, rights, soundCheck)) {
    int kRow = -1, kCol = -1;
    if (ChessLogic::FindKingCoordinates(currentTurn, boardState, kRow, kCol)) {
      DrawRectangle(boardOffsetX + (kCol * squareSize),
                    boardOffsetY + (kRow * squareSize), squareSize, squareSize,
                    palette["check_square"]);
    }
  }
}
} // namespace ChessUI

namespace ChessMode {
enum CHESSY_UTIL_MENU_MODE { CHESSY_MODE_NORMAL, CHESSY_MODE_PLAYCF };
}

namespace ChessMenu {

struct MenuUI {
  Rectangle navAbout, navHelp, navNews, navOpeningLib, navImpExport;
  Rectangle btnPlayOnline, btnPlayComputer, btnDailyPuzzles, btnPracticeMode;
};

// PURE CALCULATOR: Returns screen boundaries instantly based on live window
// size
inline MenuUI GetDynamicLayout() {
  MenuUI ui;
  float w = (float)GetScreenWidth();
  float h = (float)GetScreenHeight();

  // 1. Navigation Header Links
  ui.navAbout = {30.0f, 28.0f, 65.0f, 30.0f};
  ui.navHelp = {140.0f, 28.0f, 55.0f, 30.0f};
  ui.navNews = {230.0f, 28.0f, 55.0f, 30.0f};
  ui.navOpeningLib = {320.0f, 28.0f, 175.0f, 30.0f};
  ui.navImpExport = {530.0f, 28.0f, 125.0f, 30.0f};

  // 2. Right Sidebar Interactive Buttons
  float bX = w * 0.45f;
  float bW = w * 0.50f;
  float bH = h * 0.13f;
  float gap = h * 0.03f;
  float startY = h * 0.23f;

  ui.btnPlayOnline = {bX, startY, bW, bH};
  ui.btnPlayComputer = {bX, startY + (bH + gap), bW, bH};
  ui.btnDailyPuzzles = {bX, startY + 2 * (bH + gap), bW, bH};
  ui.btnPracticeMode = {bX, startY + 3 * (bH + gap), bW, bH};

  return ui;
}

// Handles inputs and swaps the cursor back to default if no hitboxes trip
inline void UpdateMenuInput(ChessUI::CHESSY_UI_MENU_MODE &currentMode,
                            ChessMode::CHESSY_UTIL_MENU_MODE &currentGameMode) {
  MenuUI ui = GetDynamicLayout();
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

  MenuUI ui = GetDynamicLayout();
  Vector2 mouse = GetMousePosition();

  float w = static_cast<float>(width);
  float h = static_cast<float>(height);

  // =================================================================
  // 1. DRAW ALL RAW GEOMETRY & BACKDROPS FIRST (NO SHADER)
  // =================================================================

  // Navigation Header Background
  DrawRectangle(20, 20, w - 40, 50, palette["background_dark_menu_header"]);

  // Left Side Menu Canvas Board Container
  float boardSize = h - 130.0f;
  DrawRectangle(20, 90, boardSize, boardSize,
                palette["background_dark_menu_body"]);
  DrawRectangleLinesEx({20, 90, boardSize, boardSize}, 1,
                       palette["background_dark_menu_header"]);

  // Right Side Menu Sidebar Box Container
  float panelX = 20 + boardSize + 20;
  float panelW = w - panelX - 20;
  DrawRectangle(panelX, 90, panelW, boardSize,
                palette["background_dark_menu_header"]);
  DrawRectangleLinesEx({panelX, 90, panelW, boardSize}, 1,
                       palette["background_dark_menu_body"]);

  // Render Interactive Button Panels Shape backdrops
  auto DrawButtonShape = [&](Rectangle rec) {
    bool hover = CheckCollisionPointRec(mouse, rec);
    DrawRectangleRec(rec, hover ? palette["background_dark_menu_header"]
                                : palette["background_dark_menu_body"]);
    DrawRectangleLinesEx(rec, 1,
                         hover ? palette["accent"]
                               : palette["background_dark_menu_header"]);
  };

  DrawButtonShape(ui.btnPlayOnline);
  DrawButtonShape(ui.btnPlayComputer);
  DrawButtonShape(ui.btnDailyPuzzles);
  DrawButtonShape(ui.btnPracticeMode);

  // =================================================================
  // 2. SAFE SHADER SETUP & UNIFORM DISPATCH FOR SDF FONTS
  // =================================================================

  // Query your GLSL file locations for rendering properties
  int msgTypeLoc = GetShaderLocation(sdfShader, "msgType");

  // Raylib standard configuration: type 0 tells the pipeline to parse font
  // glyph maps
  int msgType = 0;
  SetShaderValue(sdfShader, msgTypeLoc, &msgType, SHADER_UNIFORM_INT);

  // Initialize tracking logic for dynamic hovered color alterations
  auto GetLinkColor = [&](Rectangle rec) {
    return CheckCollisionPointRec(mouse, rec) ? palette["accent"]
                                              : palette["foreground_dark"];
  };

  // Turn on shader pipelines strictly for text assets
  // BeginShaderMode(sdfShader);

  // Draw Header Links Text
  DrawTextEx(font, "About", {35, 33}, 21, 1.4f, GetLinkColor(ui.navAbout));
  DrawTextEx(font, "Help", {145, 33}, 21, 1.4f, GetLinkColor(ui.navHelp));
  DrawTextEx(font, "News", {235, 33}, 21, 1.4f, GetLinkColor(ui.navNews));
  DrawTextEx(font, "Opening Library", {325, 33}, 21, 1.4f,
             GetLinkColor(ui.navOpeningLib));
  DrawTextEx(font, "Imp/Export", {535, 33}, 21, 1.4f,
             GetLinkColor(ui.navImpExport));

  // Draw Placeholder Board Container Label
  DrawTextEx(font, "[ Dynamic Menu Board ]",
             {20 + (boardSize / 4.0f), 90 + (boardSize / 2.0f) - 10}, 18, 1.0f,
             palette["foreground_dark"]);

  // Draw Button Foreground Labels
  auto DrawButtonText = [&](Rectangle rec, const char *label) {
    bool hover = CheckCollisionPointRec(mouse, rec);

    // 1. Render the container backgrounds cleanly
    DrawRectangleRec(rec, hover ? palette["background_dark_menu_header"]
                                : palette["background_dark_menu_body"]);
    DrawRectangleLinesEx(rec, 1,
                         hover ? palette["accent"]
                               : palette["background_dark_menu_header"]);

    // 2. DYNAMICALLY MEASURE TEXT EXTENTS
    // This looks at font glyph footprints to find exactly how wide/tall the
    // string is
    float fontSize = 20.0f;
    float spacing = 1.2f;
    Vector2 textSize = MeasureTextEx(font, label, fontSize, spacing);

    // 3. COMPUTE THE PERFECT CENTERED COORDINATES
    // Subtract half of the text footprint from half of the container area
    float textX = rec.x + (rec.width - textSize.x) / 2.0f;
    float textY = rec.y + (rec.height - textSize.y) / 2.0f;

    // 4. Render aligned text
    DrawTextEx(font, label, {textX, textY}, fontSize, spacing,
               hover ? palette["accent"] : palette["foreground_dark"]);
  };

  DrawButtonText(ui.btnPlayOnline, "Play Online / Friend");
  DrawButtonText(ui.btnPlayComputer, "Play vs Computer");
  DrawButtonText(ui.btnDailyPuzzles, "Daily Puzzles");
  DrawButtonText(ui.btnPracticeMode, "Self Practice Mode");

  // EndShaderMode(); // Safe deactivation
}
} // namespace ChessMenu
