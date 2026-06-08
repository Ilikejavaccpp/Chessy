// a header specifically for utility functions like rendering, piece movement,
// engine calling, etc.
#pragma once

#include <cmath>
#include <iostream>
#include <raylib.h>
#include <string>
#include <vector>

#include "core/colorscheme.h"
#include "core/dimensions.h"
#include "core/logic.h"
#include "pieces/pieces.h"

#ifndef CHESSY_UTILS_H
#define CHESSY_UTILS_H

struct ChessMouseInteraction {
  bool isDown = false; // Is the left mouse button actively held down? (crucial
                       // for dragging)
  bool dragging =
      false; // Has the user moved the cursor past the drag threshold?
  int selectedRow = -1;
  int selectedCol = -1;
  Vector2 clickStartPos = {0.0f, 0.0f};   // piece clicked coordinates
  Vector2 currentMousePos = {0.0f, 0.0f}; // live coordinates
};

// Near the top of utils.h, right under the include/structs statements:
inline int enPassantTargetRow = -1;
inline int enPassantTargetCol = -1;

namespace ChessMode {
// Forward declare this
enum CHESSY_UTIL_MENU_MODE : int;

// Explicitly tell the compiler that these enum values exist as variables
extern const CHESSY_UTIL_MENU_MODE CHESSY_MODE_NORMAL;
extern const CHESSY_UTIL_MENU_MODE CHESSY_MODE_PLAYCF;

} // namespace ChessMode

inline void loadPieceTextures(Texture2D whiteTextures[6],
                              Texture2D blackTextures[6]) {
  std::string path = "assets/textures/";
  std::string names[] = {"pawn.png", "knight.png", "bishop.png",
                         "rook.png", "queen.png",  "king.png"};

  std::cout << "[INFO] : Loading texures... -> WAIT";
  for (int i = 0; i < 6; i++) {
    whiteTextures[i] = LoadTexture((path + "w_" + names[i]).c_str());
    std::cout << "[INFO] : TEXTURES -- Loaded white texture with id "
              << whiteTextures[i].id << "\n";

    blackTextures[i] = LoadTexture((path + "b_" + names[i]).c_str());
    std::cout << "[INFO] : TEXTURES -- Loaded black texture with id "
              << blackTextures[i].id << "\n";
  }
  std::cout << "[INFO] : Finished loading textures... -> SUCCESS\n";
}

// I don't want to make the `FONT_SDF` into `FONT_DEFAULT` since we want sharp
// lines.
// MUST load the shader after this...
inline Font loadChessFont(const std::string &path) {
  // 1. Explicitly prepare the 95 printable ASCII characters (Space 32 through
  // Tilde 126) This resolves the [95/107] missing character warning from your
  // engine logs.
  int glyphCount = 95;
  int codepoints[95];
  for (int i = 0; i < glyphCount; i++) {
    codepoints[i] = 32 + i;
  }

  // 2. Load the font directly using a sharp base size
  // Raylib sets up the necessary texture buffers internally during this step
  Font font = LoadFontEx(path.c_str(), 32, codepoints, glyphCount);

  // 3. Force Bilinear filtering so the GPU maps pixel steps smoothly
  // Without this filter, custom shaders cannot render the text characters
  // properly
  SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

  std::cout << "[INFO] : Safely generated font atlas for asset: " << path
            << " -> SUCCESS\n";
  return font;
}

inline void initStartingPosition(ChessBoardMatrix &boardState) {
  // Fill the board with empty squares.
  for (int r = 0; r < boardSize; r++) {
    for (int c = 0; c < boardSize; c++) {
      boardState[r][c] = {EMPTY, NONE_COLOR};
    }
  }

  // The minor pieces (pawns)
  for (int i = 0; i < boardSize; i++) {
    boardState[1][i] = {PAWN, BLACK_PIECE};
    boardState[6][i] = {PAWN, WHITE_PIECE};
  }

  // The non-pawns
  PieceType backRowOrder[] = {ROOK, KNIGHT, BISHOP, QUEEN,
                              KING, BISHOP, KNIGHT, ROOK};
  for (int i = 0; i < boardSize; i++) {
    boardState[0][i] = {backRowOrder[i], BLACK_PIECE};
    boardState[7][i] = {backRowOrder[i], WHITE_PIECE};
  }
}

namespace ChessVisuals {
inline void DrawChessboard(const Colorscheme &palette) {
  for (int r = 0; r < boardSize; r++) {
    for (int c = 0; c < boardSize; c++) {
      // This makes it easier to read. Before, the draw function was inline (x)
      // ?:
      std::string tileKey = ((r + c) % 2 == 0) ? "square_light" : "square_dark";
      DrawRectangle(boardOffsetX + (c * squareSize),
                    boardOffsetY + (r * squareSize), squareSize, squareSize,
                    palette[tileKey]);
    }
  }
}

inline void DrawHoverHighlight(const Colorscheme &palette) {
  Vector2 mousePos = GetMousePosition();
  int col = (mousePos.x - boardOffsetX) / squareSize;
  int row = (mousePos.y - boardOffsetY) / squareSize;

  if (col >= 0 && col < boardSize && row >= 0 && row < boardSize) {
    int posX = boardOffsetX + (col * squareSize);
    int posY = boardOffsetY + (row * squareSize);
    bool isLight = (row + col) % 2 == 0;
    DrawRectangle(posX, posY, squareSize, squareSize,
                  palette[isLight ? "square_cap_light" : "square_cap_dark"]);
    DrawRectangle(posX, posY, squareSize, squareSize, palette["hover_tint"]);
    for (int i = 0; i < 3; i++) {
      DrawRectangleLines(posX + i, posY + i, squareSize - (2 * i),
                         squareSize - (2 * i), palette["accent"]);
    }
  }
}

inline void DrawGameUI(const Colorscheme &palette, Font uiFont,
                       const std::vector<PieceType> &wCaptured,
                       const std::vector<PieceType> &bCaptured,
                       const Texture2D whiteTextures[6],
                       const Texture2D blackTextures[6]) {
  int panelX = boardOffsetX + (boardSize * squareSize) + padding;
  DrawRectangle(panelX, boardOffsetY, panelWidth, boardSize * squareSize,
                palette["background_dark_menu_body"]);
  DrawRectangleLines(panelX, boardOffsetY, panelWidth, boardSize * squareSize,
                     palette["accent"]);

  DrawTextEx(uiFont, "CHESSY v1.0.3", // prev, `"CHESSY v0.1"`
             {(float)panelX + 15, (float)boardOffsetY + 20}, 20.0f, 1.0f,
             palette["accent"]);

  auto DrawCaptureThumbs = [&](const std::vector<PieceType> &list,
                               const Texture2D texArray[6], float yOff) {
    for (size_t i = 0; i < list.size(); i++) {
      int idx = (int)list[i] - 1;
      if (idx >= 0 && idx < 6 && texArray[idx].id != 0) {
        DrawTexturePro(
            texArray[idx],
            {0, 0, (float)texArray[idx].width, (float)texArray[idx].height},
            {(float)panelX + 15 + (i * 22), yOff, 22, 22}, {0, 0}, 0.0f, WHITE);
      }
    }
  };
  DrawTextEx(uiFont, "Captured by White:",
             {(float)panelX + 15, (float)boardOffsetY + 140}, 18.0f, 1.0f,
             LIGHTGRAY);
  DrawCaptureThumbs(bCaptured, blackTextures, (float)boardOffsetY + 165);
  DrawTextEx(uiFont, "Captured by Black:",
             {(float)panelX + 15, (float)boardOffsetY + 210}, 18.0f, 1.0f,
             LIGHTGRAY);
  DrawCaptureThumbs(wCaptured, whiteTextures, (float)boardOffsetY + 235);
}

inline void DrawPiecesWithHybridControls(const ChessBoardMatrix board,
                                         const Texture2D whiteTextures[6],
                                         const Texture2D blackTextures[6],
                                         const ChessMouseInteraction &state) {
  for (int r = 0; r < boardSize; r++) {
    for (int c = 0; c < boardSize; c++) {
      if (state.dragging && r == state.selectedRow && c == state.selectedCol)
        continue;
      const ChessPiece &piece = board[r][c];
      if (piece.type == EMPTY)
        continue;

      int idx = (int)piece.type - 1;
      Texture2D tex = (piece.color == WHITE_PIECE) ? whiteTextures[idx]
                                                   : blackTextures[idx];
      if (tex.id != 0) {
        DrawTexturePro(tex, {0, 0, (float)tex.width, (float)tex.height},
                       {(float)boardOffsetX + (c * squareSize),
                        (float)boardOffsetY + (r * squareSize),
                        (float)squareSize, (float)squareSize},
                       {0, 0}, 0.0f, WHITE);
      }
    }
  }
  if (state.dragging && state.selectedRow != -1) {
    const ChessPiece &piece = board[state.selectedRow][state.selectedCol];
    int idx = (int)piece.type - 1;
    Texture2D tex =
        (piece.color == WHITE_PIECE) ? whiteTextures[idx] : blackTextures[idx];
    if (tex.id != 0) {
      DrawTexturePro(tex, {0, 0, (float)tex.width, (float)tex.height},
                     {state.currentMousePos.x - (squareSize / 2.0f),
                      state.currentMousePos.y - (squareSize / 2.0f),
                      (float)squareSize, (float)squareSize},
                     {0, 0}, 0.0f, ColorAlpha(WHITE, 0.8f));
    }
  }
}

inline void DrawActiveHighlights(const Colorscheme &palette,
                                 const ChessMouseInteraction &state) {
  if (state.selectedRow != -1 && state.selectedCol != -1) {
    int posX = boardOffsetX + (state.selectedCol * squareSize);
    int posY = boardOffsetY + (state.selectedRow * squareSize);
    DrawRectangle(posX, posY, squareSize, squareSize, palette["selected_tint"]);
    DrawRectangleLines(posX, posY, squareSize, squareSize, palette["accent"]);
  }
}

inline void DrawMovePossibilityDots(const Colorscheme &palette,
                                    const std::vector<Vector2> &validMoves,
                                    const ChessBoardMatrix &board) {
  Color dotColor = ColorAlpha(BLACK, 0.25f);
  for (const auto &move : validMoves) {
    int r = (int)move.x;
    int c = (int)move.y;
    float cx = boardOffsetX + (c * squareSize) + (squareSize / 2.0f);
    float cy = boardOffsetY + (r * squareSize) + (squareSize / 2.0f);

    if (board[r][c].type == EMPTY) {
      DrawCircle(cx, cy, squareSize * 0.15f, dotColor);
    } else {
      float innerRadius = squareSize * 0.39f;
      int thickness = (int)((squareSize * 0.46f) - innerRadius);
      for (int i = 0; i < thickness; i++) {
        DrawCircleLines((int)cx, (int)cy, innerRadius + i, dotColor);
      }
    }
  }
}
} // namespace ChessVisuals

namespace ChessInput {
inline void ExecuteMoveOrCastle(int fRow, int fCol, int tRow, int tCol,
                                ChessBoardMatrix &board,
                                ChessLogic::CastlingRights &rights,
                                Sound &soundMove, Sound &soundCastle) {
  PieceType movingPiece = board[fRow][fCol].type;
  PieceColor movingColor = board[fRow][fCol].color;

  if (movingPiece == KING && std::abs(tCol - fCol) == 2) {
    int correctRow = (movingColor == WHITE_PIECE) ? 7 : 0;
    int rStart = (tCol > fCol) ? 7 : 0;
    int rEnd = (tCol > fCol) ? 5 : 3;
    board[tRow][tCol] = board[fRow][fCol];
    board[fRow][fCol] = {EMPTY, NONE_COLOR};
    board[correctRow][rEnd] = board[correctRow][rStart];
    board[correctRow][rStart] = {EMPTY, NONE_COLOR};
    PlaySound(soundCastle);
  } else {
    board[tRow][tCol] = board[fRow][fCol];
    board[fRow][fCol] = {EMPTY, NONE_COLOR};
    PlaySound(soundMove);
  }

  if (movingPiece == KING) {
    if (movingColor == WHITE_PIECE)
      rights.whiteKingMoved = true;
    else
      rights.blackKingMoved = true;
  }
  if (movingPiece == ROOK) {
    if (movingColor == WHITE_PIECE) {
      if (fRow == 7 && fCol == 7)
        rights.whiteRightRookMoved = true;
      if (fRow == 7 && fCol == 0)
        rights.whiteLeftRookMoved = true;
    } else {
      if (fRow == 0 && fCol == 7)
        rights.blackRightRookMoved = true;
      if (fRow == 0 && fCol == 0)
        rights.blackLeftRookMoved = true;
    }
  }

  // --- EN PASSANT STATE UPDATE ---
  // Default: clear the target for the next turn so the opportunity expires
  int oldTargetRow = enPassantTargetRow;
  enPassantTargetRow = -1;
  enPassantTargetCol = -1;

  // If a pawn just double-stepped, set the new ghost target square
  if (movingPiece == PAWN) {
    if (fRow == 6 && tRow == 4) { // White pawn double-step
      enPassantTargetRow = 5;     // The square it skipped over
      enPassantTargetCol = fCol;
    } else if (fRow == 1 && tRow == 3) { // Black pawn double-step
      enPassantTargetRow = 2;            // The square it skipped over
      enPassantTargetCol = fCol;
    }
  }
}

inline void ProcessDualInput(ChessBoardMatrix &board, PieceColor &currentTurn,
                             ChessMouseInteraction &state,
                             std::vector<PieceType> &wCaptured,
                             std::vector<PieceType> &bCaptured,
                             ChessLogic::CastlingRights &rights,
                             Sound soundCapture, Sound soundCastle,
                             Sound soundCheck, Sound soundMove,
                             int &epTargetRow, int &epTargetCol) {
  Vector2 mouse = GetMousePosition();
  state.currentMousePos = mouse;
  int col = (mouse.x - boardOffsetX) / squareSize;
  int row = (mouse.y - boardOffsetY) / squareSize;
  bool inside = (col >= 0 && col < boardSize && row >= 0 && row < boardSize);

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && inside) {
    state.isDown = true;
    state.clickStartPos = mouse;

    if (state.selectedRow == -1) {
      if (board[row][col].type != EMPTY &&
          board[row][col].color == currentTurn) {
        state.selectedRow = row;
        state.selectedCol = col;
      }
    } else {
      if (row == state.selectedRow && col == state.selectedCol) {
        state.selectedRow = -1;
        state.selectedCol = -1;
      } else if (board[row][col].type != EMPTY &&
                 board[row][col].color == currentTurn) {
        state.selectedRow = row;
        state.selectedCol = col;
      } else {
        if (ChessLogic::IsMoveAbsolutelyLegal(state.selectedRow,
                                              state.selectedCol, row, col,
                                              currentTurn, board, rights)) {
          bool isEnPassant =
              (board[state.selectedRow][state.selectedCol].type == PAWN) &&
              (col == epTargetCol) && (row == epTargetRow);

          if (board[row][col].type != EMPTY || isEnPassant) {
            if (isEnPassant) {
              // 1. Identify where the enemy pawn is actually sitting
              int victimRow = (currentTurn == WHITE_PIECE) ? row + 1 : row - 1;

              // 2. Add the captured enemy pawn to the CORRECT player's sidebar
              // list
              if (currentTurn == WHITE_PIECE) {
                bCaptured.push_back(PAWN); // White captures Black's pawn
              } else {
                wCaptured.push_back(PAWN); // Black captures White's pawn
              }

              // 3. Vaporize the enemy pawn from its physical square
              board[victimRow][col] = {EMPTY, NONE_COLOR};
            } else {
              if (board[row][col].color == WHITE_PIECE)
                wCaptured.push_back(board[row][col].type);
              else
                bCaptured.push_back(board[row][col].type);
            }
            PlaySound(soundCapture); // both are classified as captures.
          } else {
            PlaySound(soundMove);
          }
          ExecuteMoveOrCastle(state.selectedRow, state.selectedCol, row, col,
                              board, rights, soundMove, soundCastle);
          currentTurn =
              (currentTurn == WHITE_PIECE) ? BLACK_PIECE : WHITE_PIECE;
        }
        state.selectedRow = -1;
        state.selectedCol = -1;
      }
    }
  }

  if (state.isDown && IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
      state.selectedRow != -1) {
    float dx = mouse.x - state.clickStartPos.x;
    float dy = mouse.y - state.clickStartPos.y;
    if ((dx * dx + dy * dy) > 100.0f)
      state.dragging = true;
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && state.isDown) {
    state.isDown = false;
    if (state.dragging) {
      state.dragging = false;
      if (inside && !(row == state.selectedRow && col == state.selectedCol)) {
        if (ChessLogic::IsMoveAbsolutelyLegal(state.selectedRow,
                                              state.selectedCol, row, col,
                                              currentTurn, board, rights)) {
          bool isEnPassant =
              (board[state.selectedRow][state.selectedCol].type == PAWN) &&
              (col == epTargetCol) && (row == epTargetRow);

          if (board[row][col].type != EMPTY || isEnPassant) {
            if (isEnPassant) {
              // 1. Identify where the enemy pawn is actually sitting
              int victimRow = (currentTurn == WHITE_PIECE) ? row + 1 : row - 1;

              // 2. Add the captured enemy pawn to the CORRECT player's sidebar
              // list
              if (currentTurn == WHITE_PIECE) {
                bCaptured.push_back(PAWN); // White captures Black's pawn
              } else {
                wCaptured.push_back(PAWN); // Black captures White's pawn
              }

              // 3. Vaporize the enemy pawn from its physical square
              board[victimRow][col] = {EMPTY, NONE_COLOR};
            } else {
              if (board[row][col].color == WHITE_PIECE)
                wCaptured.push_back(board[row][col].type);
              else
                bCaptured.push_back(board[row][col].type);
            }
            PlaySound(soundCapture);
          } else {
            PlaySound(soundMove);
          }
          ExecuteMoveOrCastle(state.selectedRow, state.selectedCol, row, col,
                              board, rights, soundMove, soundCastle);
          currentTurn =
              (currentTurn == WHITE_PIECE) ? BLACK_PIECE : WHITE_PIECE;
          state.selectedRow = -1;
          state.selectedCol = -1;
        }
      }
    }
  }
}
} // namespace ChessInput

namespace Vendor::ypkg {

// This function is a handy dandy
// Starts at 1 to skip the executable name, returns
// - the index (`i`) if found, param `ret_index` == true
// - the value of 0 (if found) or 1 (otherwise), param `ret_index` == false
inline int findOption(char **list, int listSize, std::string_view targetOption,
                      bool ret_index = false) {
  if (ret_index)
    for (int i = 1; i < listSize; ++i) {
      if (list[i] == nullptr)
        continue; // Safety check

      std::string_view arg(list[i]);

      // Matches exact flag (e.g., "--cli")
      if (arg == targetOption) {
        return i;
      }

      // Matches flag with an embedded '=' (e.g., "--mode=cli")
      if (arg.rfind(targetOption, 0) == 0 && arg.size() > targetOption.size() &&
          arg[targetOption.size()] == '=') {
        return i;
      }
    }

  else
    for (int i = 1; i < listSize; ++i) {
      if (list[i] == nullptr)
        continue; // Safety check

      std::string_view arg(list[i]);

      // Matches exact flag (e.g., "--cli")
      if (arg == targetOption) {
        return 0;
      }

      // Matches flag with an embedded '=' (e.g., "--mode=cli")
      if (arg.rfind(targetOption, 0) == 0 && arg.size() > targetOption.size() &&
          arg[targetOption.size()] == '=') {
        return 0;
      }
    }

  return -1;
}
} // namespace Vendor::ypkg

// ChessMenu for `menu.h`, specifically the options
namespace ChessMenu::logic {
inline void
UpdateMenuElement(int width, int height, Vector2 mouse,
                  enum ChessMode::CHESSY_UTIL_MENU_MODE currentScreenMode) {
  // Mirror the layout metrics utilized inside the DrawStartMenu rendering
  // canvas
  int boardSize = height - 130;
  int panelX = 20 + boardSize + 20;
  int panelW = width - panelX - 20;
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
    currentScreenMode =
        ChessMode::CHESSY_MODE_PLAYCF; // Slide application into gameplay loop
  }
}
} // namespace ChessMenu::logic

#endif
