#pragma once

// COLORSCHEME
// #define CHESSY_COLROSCHEME_ARC_SOFT

#include <raylib.h>

#include "backend/clayb_util.h"
#include "core/colorscheme.h"
#include "core/dimensions.h"
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

      if (ChessLogic::GetKingMBCState(currentTurn, boardState, rights) ==
              "checkmate" ||
          ChessLogic::GetKingMBCState(currentTurn, boardState, rights) ==
              "stalemate") {

        // Return to the home page
        if (hasPlayedSound == true &&
            ChessLogic::GetKingMBCState(currentTurn, boardState, rights) ==
                "checkmate") { // since this will be reset, hence once.
          std::cout << "[INFO] : LOGIC -- Checkmate, returning to HOME. -> "
                       "REDIRECT\n";
        } else {
          std::cout << "[INFO] : LOGIC -- Stalemate: aka a draw, returning to "
                       "HOME. -> "
                       "REDIRECT\n";
        }
        // Add some score saving here -> FUTURE
        // Add some delay or input here -> FUTURE not far
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

  SetMouseCursor(isHovering ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_ARROW);

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

inline void DrawAboutMenu(Colorscheme &palette, Font &font, Font &font_reg,
                          int width, int height, Shader &sdfShader) {
  ClearBackground(palette["background_dark"]);

  float w = static_cast<float>(width);
  float h = static_cast<float>(height);

  // ALIGNED: Feeds identical frame floats directly into the drawer engine
  // layout pass
  MenuUI ui = GetDynamicLayout(w, h);
  Vector2 mouse = GetMousePosition();
  Vector2 wheel = GetMouseWheelMoveV();
  header_padding pad;

  auto InitHeaderPadding = [&pad]() -> void {
    pad.about_text_up = 40;
    pad.about_text_down = 40;
    pad.about_text_left = 30;
    pad.about_text_right = 30;

    pad.about_container_main_right = 20;
    pad.about_container_main_down = 40;
    pad.about_container_main_up = 40;
    pad.about_container_main_left = 20;
  };

  InitHeaderPadding();

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
  // 1. Calculate relative container width and height
  float containerX = pad.about_container_main_left;
  float containerY = pad.about_container_main_up + 50.0f;
  float containerW =
      w - pad.about_container_main_left - pad.about_container_main_right;
  float containerH = h - containerY - pad.about_container_main_down;

  // 2. Draw the container
  DrawRectangle(containerX, containerY, containerW, containerH,
                palette["background_dark_menu_body"]);

  // 3. Define the Inner Text Viewport Box
  float contentX = containerX + pad.about_text_left;
  float contentY = containerY + pad.about_text_up;
  float contentW = containerW - pad.about_text_left - pad.about_text_right;

  // Set this to how long the paragraph physically takes up inside the textbox
  // (e.g. 650px)
  const char *aboutText =
      "Brief (ai) generated summary:\n"
      "Chessy is a high-performance custom chess client crafted with modern "
      "C++ "
      "and Raylib. It features tactical hybrid controls, real-time move "
      "validation, "
      "SDF-filtered text displays, and integrated tools to review openings or "
      "solve dynamic puzzles."
      "\n"
      "\n"
      "Build info:\n"
      "Version -- v1.0.4 Chessy v_si_main_1branch_04\n"
      "Status  -- Stable\n"
      "Includes-- UI improvements";
  float estimatedContentHeight = Vendor::clay::GetEstimatedContentHeight(
      font_reg, (const char *[]){aboutText}, 1, contentW, 18.0f, 1.3f);
  float contentH = containerH - pad.about_text_up - pad.about_text_down;

  // Draw the text here
  // First some settings
  Clay_SetPointerState(Clay_Vector2{mouse.x, mouse.y},
                       IsMouseButtonDown(MOUSE_BUTTON_LEFT));
  Clay_UpdateScrollContainers(
      false, // Set to false to disable touch/drag
             // scrolling if you only want wheel
             // CHANGE -- changed to `true` from `false`. You can now
             // drag the wheel.
             // CHANGE -- turns  out, the prev change was false.
      Clay_Vector2{wheel.x * 35.0f, wheel.y * 35.0f},
      GetFrameTime() // Clean, normalized timing parameter
                     // this is so that it refreshes at the *CORRECT*
                     // refresh rate.
  );

  // 1. Set the internal text wrapping boundaries
  Rectangle textBounds = {contentW, contentH, contentW,
                          estimatedContentHeight}; // for clay

  // 2. Trigger Clay to render stuff: from the c obj file in the `bin/` folder.
  // Refer to file @file`src/clay_impl.c`
  float scrollOffsetY = BuildChessyMenuAboutPage(
      contentW, contentH, estimatedContentHeight, GetFrameTime());
  // Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(
  //     Clay_GetElementId(CLAY_STRING("TextBoxScrollRegion"))); // math stuff
  struct {
    bool found = true;
  } scrollData; // mock it

  // 3. Render wrapped paragraphs safely.

  if (scrollData.found) {
    // Extract the live floating point Y-offset from the scrollPosition vector
    // field is already done by the function and is mocked.
    // so just update the text bounds.

    // make it less choppy
    int renderX = (int)std::floor(contentX);
    int renderY = (int)std::floor(contentY);
    int renderW = (int)std::floor(contentW);
    int renderH = (int)std::floor(contentH);

    textBounds = {contentX, renderY + scrollOffsetY, contentW,
                  estimatedContentHeight};

    // Make sure we clip overflowing ui stuff thus for a smooth scroll.
    BeginScissorMode(renderX, renderY, renderW, renderH);

    Vendor::raylib::DrawTextBoxed(font_reg, aboutText, textBounds, 18.0f, 1.3f,
                                  true, palette["foreground_dark"]);

    EndScissorMode();

    // Track the scrolling and draw the scroll bar
    float barTrackX = contentX + contentW - 8.0f;
    DrawRectangle(barTrackX, contentY, 6, contentH,
                  palette["background_dark_scrollbar"]);

    float maxScrollableWindowOffset =
        estimatedContentHeight -
        contentH; // this is a bound so that we  don't get a
                  // segfault. its like the box in which your mouse can't go any
                  // further (in your computer, phone, tab, etc.)

    if (maxScrollableWindowOffset > 0) // no bugs
    {
      float thumbH = (contentH * contentH / estimatedContentHeight);

      // Map the calculation directly to the raw scrollOffsetY value
      float scrollRatio = (-scrollOffsetY) / maxScrollableWindowOffset;
      float thumbY = contentY + (scrollRatio * (contentH - thumbH));

      // Draw it
      DrawRectangle(barTrackX, thumbY, 6, thumbH,
                    palette["background_dark_scrollbar_current"]);
    }
  }
}

inline void DrawHelpMenu(Colorscheme &palette, Font &font, Font &font_reg,
                         int width, int height, Shader &sdfShader) {
  ClearBackground(palette["background_dark"]);

  float w = static_cast<float>(width);
  float h = static_cast<float>(height);

  // ALIGNED: Feeds identical frame floats directly into the drawer engine
  // layout pass
  MenuUI ui = GetDynamicLayout(w, h);
  Vector2 mouse = GetMousePosition();
  Vector2 wheel = GetMouseWheelMoveV();
  header_padding pad;

  auto InitHeaderPadding = [&pad]() -> void {
    pad.about_text_up = 40;
    pad.about_text_down = 40;
    pad.about_text_left = 30;
    pad.about_text_right = 30;

    pad.about_container_main_right = 20;
    pad.about_container_main_down = 40;
    pad.about_container_main_up = 40;
    pad.about_container_main_left = 20;
  };

  InitHeaderPadding();

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
  // 1. Calculate relative container width and height
  float containerX = pad.about_container_main_left;
  float containerY = pad.about_container_main_up + 50.0f;
  float containerW =
      w - pad.about_container_main_left - pad.about_container_main_right;
  float containerH = h - containerY - pad.about_container_main_down;

  // 2. Draw the container
  DrawRectangle(containerX, containerY, containerW, containerH,
                palette["background_dark_menu_body"]);

  // 3. Define the Inner Text Viewport Box
  float contentX = containerX + pad.about_text_left;
  float contentY = containerY + pad.about_text_up;
  float contentW = containerW - pad.about_text_left - pad.about_text_right;

  // Set this to how long the paragraph physically takes up inside the textbox
  // (e.g. 650px)
  const unsigned char topic_header_no = 2;

  // The `2` here in the lhs of the addition is a magic number.
  // It corresponds to the Headers beginning, section.
  // You can modify the section string and put your customs (with correct count)
  // at the end BUT please do not
  // *modify* the magic number.

  // this is too dangerous
  // DANGEROUS, DEPECRATED, FIX
  // const char *aboutText[2 + int(topic_header_no * 2)] = {
  //     // Headers, beginning
  //     "Help manual...\n"
  //     "\n"
  //     "Scroll down to see more...\n",
  //
  //     // The about like introduction.
  //     "You may be wondering... what brings us here\n"
  //     "You may have clicked the `Help` option searching for help on how to
  //     use " "this app. You've come to the right place.\n" "Scroll down up to
  //     the `Help with Chessy:` section to see more.\n"
  //     "\n"
  //     "\n"
  //     "\n"
  //     "\n",
  //
  //     /*
  //      * Your own additions go down here. Pls update the count.
  //      * The topic_header_no will be the amount of bodies OR headings (Like
  //      * above)
  //      * below this comment.
  //      */
  //
  //     // the help page for chessy...
  //
  //     // Heading. INDEX -> (3-1)==>2
  //     "Help with Chessy:\n"
  //     "\n",
  //
  //     // Body. INDEX -> (4-1) ==>3
  //     "Chessy is a free to-play and NOT pay-to-win (p2w) chess app built
  //     using " "raylib and clay. It is a collection of the `yapps` suite. To
  //     use it, " "just click around on the buttons to navigate.\n"
  //     "\n"
  //     "To navigate back to where you left off...\n"
  //     "  - If it is the Home menu, then click again to the page where you had
  //     "
  //     "\n"
  //     "previously clicked.\n"
  //     " Do note that we are gonna add a visual appearance change to indicate
  //     \n" "where one left off.\n" "  - If it is not the Home menu (e.g. this
  //     Help menu or the About menu), "
  //     "\n"
  //     "then click the previously or desired menu.\n"
  //     "\n"
  //     "\n"
  //     "\n",
  //
  //     // help page for general chess
  //
  //     // Heading. INDEX -> (5-1)==>4
  //     "Help with Chess:\n"
  //     "\n",
  //
  //     // Body. INDEX -> (6-1)==>5
  //     "If you are currently online, then go visit the wiki or an web page for
  //     " "how to play chess.\n" "If you are currently offline (best guess),
  //     then please consider the " "advice of a \"beginner to expert\" type
  //     chess book.\n"
  //     "\n"
  //     "NOTE: Support or Addition of chessy's own simplified way (in plain "
  //     "english) about the rules of chess will come soon (v1.0.5 sneak peek)"
  //
  //     // END
  // };
  const TextBlockProfile aboutText[] = {
      // Headers-Body layout
      {.text = "Help Manual..." CHESSY_CSS_MENU__header_body_sep,
       .type = TextBlockType::text_Header},
      {.text = "Scroll down to see more..." CHESSY_CSS_MENU__header_body_sep,
       .type = TextBlockType::text_Header},
      {.text = "Help with chessy", .type = TextBlockType::text_Header},
      {.text = ""
               "Lorem ipsum\n"
               "More filler text!\n"
               "Add the shihh here.\n"

       ,
       .type = TextBlockType::text_Header}};
  // Change the type to `unsigned int` or `uint16` if you want 65k paragraphs.
  const unsigned short totalElements = sizeof(aboutText) / sizeof(aboutText[0]);

  float estimatedContentHeight = 0.0f;
  // OLD version
  // for (unsigned char i = 0; i < (2 + (2 * topic_header_no)); ++i) {
  //   float currentFontSize = (i % 2 == 0) ? 24.0f : 18.0f;
  //   float blockHeight = Vendor::clay::GetEstimatedContentHeight(
  //       font_reg, (const char *[]){aboutText[i].text}, 1, contentW,
  //       currentFontSize, 1.3f);
  //   float elementPaddingGap =
  //       (i % 2 == 0) ? 12.0f : 24.0f; // Synchronized back to 12.0f
  //   estimatedContentHeight += (blockHeight + elementPaddingGap);
  // }
  // float contentH = containerH - pad.about_text_up - pad.about_text_down;

  for (unsigned char i = 0; i < totalElements; ++i) {
    float currentFontSize =
        (aboutText[i].type == TextBlockType::text_Header) ? 24.0f : 18.0f;
    float elementPaddingGap = (aboutText[i].type == TextBlockType::text_Header)
                                  ? 12.0f
                                  : 24.0f; // Synchronized back to 12.0f

    float blockHeight = Vendor::clay::GetEstimatedContentHeight(
        font_reg, (const char *[]){aboutText[i].text}, 1, contentW,
        currentFontSize, 1.3f);
    // float elementPaddingGap =
    //     (i % 2 == 0) ? 12.0f : 24.0f; // Synchronized back to 12.0f
    estimatedContentHeight += (blockHeight + elementPaddingGap);
  }
  estimatedContentHeight += 16.0f; // Structural buffer safeguard
  float contentH = containerH - pad.about_text_up - pad.about_text_down;

  // Draw the text here
  // First some settings
  Clay_SetPointerState(Clay_Vector2{mouse.x, mouse.y},
                       IsMouseButtonDown(MOUSE_BUTTON_LEFT));
  Clay_UpdateScrollContainers(
      false, // Set to false to disable touch/drag
             // scrolling if you only want wheel
             // CHANGE -- changed to `true` from `false`. You can now
             // drag the wheel.
             // CHANGE -- turns  out, the prev change was false.
      Clay_Vector2{wheel.x * 35.0f, wheel.y * 35.0f},
      GetFrameTime() // Clean, normalized timing parameter
                     // this is so that it refreshes at the *CORRECT*
                     // refresh rate.
  );

  // 1. Set the internal text wrapping boundaries
  Rectangle textBounds = {contentW, contentH, contentW,
                          estimatedContentHeight}; // for clay

  // 2. Trigger Clay to render stuff: from the c obj file in the `bin/` folder.
  // Refer to file @file`src/clay_impl.c`
  float scrollOffsetY = BuildChessyMenuHelpPage(
      contentW, contentH, estimatedContentHeight, GetFrameTime());
  // Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(
  //     Clay_GetElementId(CLAY_STRING("TextBoxScrollRegion"))); // math stuff
  struct {
    bool found = true;
  } scrollData;

  // 3. Render wrapped paragraphs safely.

  if (scrollData.found) {
    // Extract the live floating point Y-offset from the scrollPosition vector
    // field is already done by the function and is mocked.
    // so just update the text bounds.

    // make it less choppy
    int renderX = (int)std::floor(contentX);
    int renderY = (int)std::floor(contentY);
    int renderW = (int)std::floor(contentW);
    int renderH = (int)std::floor(contentH);

    textBounds = {contentX, renderY + scrollOffsetY, contentW,
                  estimatedContentHeight};

    // Alternative 1

    // Make sure we clip overflowing ui stuff thus for a smooth scroll.
    {
      BeginScissorMode(renderX, renderY, renderW, renderH);

      const float baselineContentW = (float)contentW;

      // Calculate layout advancements starting from a 0-indexed local
      // accumulator
      float localLayoutAccumulatorY = 0.0f;

      for (unsigned char i = 0; i < totalElements; ++i) {
        float currentFontSize = (i % 2 == 0) ? 24.0f : 18.0f;

        // Fresh content profiling on every single resize update tick
        float trueBlockHeight = Vendor::clay::GetEstimatedContentHeight(
            font_reg, (const char *[]){aboutText[i].text}, 1, baselineContentW,
            currentFontSize, 1.3f);

        // Inject the layout offset ONLY during the immediate viewport mapping
        // pass

        // DEPECRATED
        // float finalRenderY =
        //     (float)((int)(renderY + scrollOffsetY +
        //     localLayoutAccumulatorY));
        //
        // Rectangle itemBounds = {(float)((int)contentX), finalRenderY,
        //                         (float)((int)baselineContentW),
        //                         (float)((int)trueBlockHeight)};
        float finalRenderY =
            std::floor(renderY + scrollOffsetY + localLayoutAccumulatorY);
        float snappedHeight = std::floor(trueBlockHeight);

        Rectangle itemBounds = {std::floor(contentX), finalRenderY,
                                std::floor(baselineContentW), snappedHeight};

        Vendor::raylib::DrawTextBoxed(font_reg, aboutText[i].text, itemBounds,
                                      currentFontSize, 1.3f, true,
                                      palette["foreground_dark"]);

        // Adjust padding rules on the fly to prevent headers from blending into
        // previous text fields
        float elementPaddingGap = (i % 2 == 0) ? 12.0f : 24.0f;
        localLayoutAccumulatorY += (trueBlockHeight + elementPaddingGap);
      }

      EndScissorMode();
    }

    // Alternative 2
    //
    // // Make sure we clip overflowing ui stuff thus for a smooth scroll.
    // {
    //   BeginScissorMode(renderX, renderY, renderW, renderH);
    //
    //   // 1. Establish an invariant layout width baseline.
    //   // Do NOT let scroll operations mutate this width context.
    //   // Please, you don't know the amount of ui bugs I faced with this block
    //   const float fixedContentW = (float)contentW;
    //
    //   // 2. Start our accumulation loop from a pure baseline (0),
    //   // decoupling text flow math completely from the raw scroll context.
    //   float currentLocalY = 0.0f;
    //
    //   for (unsigned char i = 0; i < (2 + (2 * topic_header_no)); ++i) {
    //     // Explicit parameters matching font choices
    //     float currentFontSize = (i % 2 == 0) ? 24.0f : 18.0f;
    //
    //     // Force calculations to evaluate based on consistent structural
    //     // measurements
    //     float blockHeight = Vendor::clay::GetEstimatedContentHeight(
    //         font_reg, (const char *[]){aboutText[i]}, 1, fixedContentW,
    //         currentFontSize, 1.3f);
    //
    //     // 3. Inject scrollOffsetY ONLY when mapping the final drawing
    //     geometry Rectangle itemBounds = {contentX,
    //                             (float)renderY + scrollOffsetY +
    //                             currentLocalY, fixedContentW, blockHeight};
    //
    //     Vendor::raylib::DrawTextBoxed(font_reg, aboutText[i], itemBounds,
    //                                   currentFontSize, 1.3f, true,
    //                                   palette["foreground_dark"]);
    //
    //     // Accumulate height calculations in a clean coordinate system
    //     currentLocalY +=
    //         (blockHeight + 14.0f); // Added 14.0f uniform layout padding
    //   }
    //
    //   EndScissorMode();
    // }

    // Track the scrolling and draw the scroll bar
    float barTrackX = contentX + contentW - 8.0f;
    DrawRectangle(barTrackX, contentY, 6, contentH,
                  palette["background_dark_scrollbar"]);

    float maxScrollableWindowOffset =
        estimatedContentHeight -
        contentH; // this is a bound so that we  don't get a
                  // segfault. its like the box in which your mouse can't go any
                  // further (in your computer, phone, tab, etc.)

    if (maxScrollableWindowOffset > 0) // no bugs
    {
      float thumbH = (contentH * contentH / estimatedContentHeight);

      // Map the calculation directly to the raw scrollOffsetY value
      float scrollRatio = (-scrollOffsetY) / maxScrollableWindowOffset;
      float thumbY = contentY + (scrollRatio * (contentH - thumbH));

      // Draw it
      DrawRectangle(barTrackX, thumbY, 6, thumbH,
                    palette["background_dark_scrollbar_current"]);
    }
  }
}

} // namespace ChessMenu
