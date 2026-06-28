// a header specifically for utility functions like rendering, piece movement,
// engine calling, etc.
#pragma once

#include <cmath>
#include <iostream>
#include <raylib.h>
#include <string>
#include <string_view> // for high optimizations...
#include <vector>

#include "backend/smolInt.hpp"
#include "core/board.h"
#include "core/colorscheme.h"
#include "core/dimensions.h"
#include "core/logic.h"
#include "pieces/pieces.h"

#ifndef CHESSY_UTILS_H
#define CHESSY_UTILS_H

// these are for text related stuff, since the padding keeps breaking
#if defined(__cplusplus) && defined(CHESSY_CONFIG_USE_COMPAT_NONC)

enum class TextBlockType {
  text_Header,
  text_Body // no need for footer since we can use stuff.
};

struct TextBlockProfile {
private:
  const char *text;
  TextBlockType type;

public:
  void setType(TextBlockType type) { this->type = type; }

  void setText(const char *text) { this->text = text; }
};

#endif

// the nice newline sep.
#define CHESSY_CSS_MENU__header_body_sep "\n\n"

// TextBlockType
enum class TextBlockType {
  text_Header,
  text_Body // no need for footer since we can use stuff.
};

struct TextBlockProfile {
  const char *text;
  TextBlockType type;
};

#ifdef __cplusplus
namespace ChessMenu::text {
#endif

// C compatibility
inline void setType(TextBlockProfile *__this, TextBlockType type) {
  __this->type = type;
}

inline void setText(TextBlockProfile *__this, const char *text) {
  __this->text = text;
}

#ifdef __cplusplus
} // namespace ChessMenu::text
#endif

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
  CHESSY_MODE_ONLINE,
  CHESSY_MODE_PUZZLE
};
}

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

// For pawn promotion.
struct PromotionState {
  bool active = false; // is it ready to promote? // is it the promotion stage
  int row =
      -1; // row (rank, 8th or `row=0` for white; 1st or `row=7` for black)
  int col = -1; // col (file)
  PieceColor color =
      NONE_COLOR; // the turn. THIS IS UNINITIALIZED SINCE WE DON'T KNOW WHO
                  // PROMOTED THEIR PAWNS AT THE START OF THE GAME
};

// Near the top of utils.h, right under the include/structs statements:
inline int enPassantTargetRow = -1;
inline int enPassantTargetCol = -1;

inline PromotionState activePromotion; // since they are defaulted.
inline Colorscheme palette;

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

// Here `tRow` and `tCol` mean the current (this) row and col
// of the pawn. The `color` param is for the current color (w/b) or the current
// 'turn'
inline void DrawPawnPromotionUI(int tRow, int tCol, PieceColor &color,
                                const Texture2D whiteTextures[6],
                                const Texture2D blackTextures[6],
                                Vector2 &mousePos, Colorscheme &palette) {
  // Nice dimmed board.
  // Dim the board to a nice 60% light.
  DrawRectangle(boardOffsetX, boardOffsetY, boardSize * squareSize,
                boardSize * squareSize, ColorAlpha(BLACK, 0.4));

  // --- MENU section ---
  // The dimensions
  float menuX = (float)boardOffsetX + (tCol * squareSize);
  float menuWidth = (float)squareSize;
  float menuHeight = (float)squareSize * 4.0f;

  // Pivot placement layout depending on which side is promoting
  // White promotes moving up (lands on Row 0 -> drops panel down rows 0,1,2,3)
  // Black promotes moving down (lands on Row 7 -> sprouts panel up rows
  // 7,6,5,4)
  float menuY = (color == WHITE_PIECE)
                    ? (float)boardOffsetY + (squareSize)
                    : (float)boardOffsetY + ((tRow - 3) * squareSize);

  // Draw the menu
  DrawRectangleRec({menuX, menuY, menuWidth, menuHeight},
                   palette["background_dark_promotion_menu_body"]);
  DrawRectangleLinesEx({menuX, menuY, menuWidth, menuHeight}, 2.0f,
                       palette["background_dark_menu_header"]);

  // Draw the selection menu with textures (n, b, r, q)
  PieceType promotionCandidates[4] = {KNIGHT, BISHOP, ROOK, QUEEN};

  // ya know what. frick `int i` use memory shit
  for (unsigned char i = 0; i < 4; ++i) {

    // Item size and bounds
    float itemY = menuY + (i * squareSize);
    Rectangle itemBounds = {menuX, itemY, (float)squareSize, (float)squareSize};

    // Check if the mouse is hovering over any of them.
    if (CheckCollisionPointRec(mousePos, itemBounds)) {
      DrawRectangleRec(itemBounds, palette["hover_button"]);
      DrawRectangleLinesEx(itemBounds, 1.0f, palette["hover_button_outline"]);
    }

    // Map choice enum elements back into texture asset IDs (Enum Type Val - 1)
    int idx = (int)promotionCandidates[int(i)] - 1;
    Texture2D pieceTex =
        (color == WHITE_PIECE) ? whiteTextures[idx] : blackTextures[idx];

    // Safely draw the piece centered perfectly inside its menu tile box
    if (pieceTex.id != 0) {
      DrawTexturePro(
          pieceTex, {0.0f, 0.0f, (float)pieceTex.width, (float)pieceTex.height},
          itemBounds, {0.0f, 0.0f}, 0.0f, WHITE);
    }
  }
}
} // namespace ChessVisuals

namespace ChessInput {
inline void ProcessPromotionSelection(ChessBoardMatrix &board,
                                      const Vector2 &mousePos,
                                      Sound &soundMove) {
  if (!activePromotion.active)
    return;

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    // Exact layout boundary logic mirrored from the rendering engine setup
    float menuX = (float)boardOffsetX + (activePromotion.col * squareSize);
    float menuWidth = (float)squareSize;
    float menuY =
        (activePromotion.color == WHITE_PIECE)
            ? (float)boardOffsetY + (squareSize)
            : (float)boardOffsetY + ((activePromotion.row - 3) * squareSize);

    PieceType promotionCandidates[4] = {KNIGHT, BISHOP, ROOK, QUEEN};

    for (unsigned char i = 0; i < 4; ++i) {
      float itemY = menuY + (i * squareSize);
      Rectangle itemBounds = {menuX, itemY, menuWidth, (float)squareSize};

      if (CheckCollisionPointRec(mousePos, itemBounds)) {
        // Mutate board matrix position to target choice selection piece
        board[activePromotion.row][activePromotion.col] = {
            promotionCandidates[i], activePromotion.color};

        // Deactivate promotion cycle states
        activePromotion.active = false;
        activePromotion.row = -1;
        activePromotion.col = -1;
        activePromotion.color = NONE_COLOR;

        PlaySound(soundMove);
        break;
      }
    }
  }
}

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

  // --- PAWN PROMOTION UPDATE ---
  if (movingPiece == PAWN) {
    if ((movingColor == WHITE_PIECE && tRow == 0) ||
        (movingColor == BLACK_PIECE && tRow == 7)) {
      activePromotion.active = true;
      activePromotion.row = tRow;
      activePromotion.col = tCol;
      activePromotion.color = movingColor;
    }
  }
}

inline void ProcessDualInput(ChessBoardMatrix &board, PieceColor &currentTurn,
                             ChessMouseInteraction &state,
                             std::vector<PieceType> &wCaptured,
                             std::vector<PieceType> &bCaptured,
                             ChessLogic::CastlingRights &rights,
                             Sound &soundCapture, Sound &soundCastle,
                             Sound &soundCheck, Sound &soundMove,
                             int &epTargetRow, int &epTargetCol) {
  Vector2 mouse = GetMousePosition();
  state.currentMousePos = mouse;
  int col = (mouse.x - boardOffsetX) / squareSize;
  int row = (mouse.y - boardOffsetY) / squareSize;
  bool inside = (col >= 0 && col < boardSize && row >= 0 && row < boardSize);

  if (activePromotion.active) {
    // make an update for this.
    ProcessPromotionSelection(board, mouse, soundMove);
    return;
  }

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

namespace Vendor::raylib {

// Draw text using font inside rectangle limits with support for text selection
inline void DrawTextBoxedSelectable(Font font, const char *text, Rectangle rec,
                                    float fontSize, float spacing,
                                    bool wordWrap, Color tint, int selectStart,
                                    int selectLength, Color selectTint,
                                    Color selectBackTint) {
  int length = TextLength(
      text); // Total length in bytes of the text, scanned by codepoints in loop

  float textOffsetY = 0;    // Offset between lines (on line break '\n')
  float textOffsetX = 0.0f; // Offset X to next character to draw

  float scaleFactor =
      fontSize / (float)font.baseSize; // Character rectangle scaling factor

  // Word/character wrapping mechanism variables
  enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
  int state = wordWrap ? MEASURE_STATE : DRAW_STATE;

  int startLine = -1; // Index where to begin drawing (where a line begins)
  int endLine = -1;   // Index where to stop drawing (where a line ends)
  int lastk = -1;     // Holds last value of the character position

  for (int i = 0, k = 0; i < length; i++, k++) {
    // Get next codepoint from byte string and glyph index in font
    int codepointByteCount = 0;
    int codepoint = GetCodepoint(&text[i], &codepointByteCount);
    int index = GetGlyphIndex(font, codepoint);

    // NOTE: Normally we exit the decoding sequence as soon as a bad byte is
    // found (and return 0x3f) but we need to draw all of the bad bytes using
    // the '?' symbol moving one byte
    if (codepoint == 0x3f)
      codepointByteCount = 1;
    i += (codepointByteCount - 1);

    float glyphWidth = 0;
    if (codepoint != '\n') {
      glyphWidth = (font.glyphs[index].advanceX == 0)
                       ? font.recs[index].width * scaleFactor
                       : font.glyphs[index].advanceX * scaleFactor;

      if (i + 1 < length)
        glyphWidth = glyphWidth + spacing;
    }

    // NOTE: When wordWrap is ON we first measure how much of the text we can
    // draw before going outside of the rec container We store this info in
    // startLine and endLine, then we change states, draw the text between those
    // two variables and change states again and again recursively until the end
    // of the text (or until we get outside of the container) When wordWrap is
    // OFF we don't need the measure state so we go to the drawing state
    // immediately and begin drawing on the next line before we can get outside
    // the container
    if (state == MEASURE_STATE) {
      // TODO: There are multiple types of spaces in UNICODE, maybe it's a good
      // idea to add support for more Ref: http://jkorpela.fi/chars/spaces.html
      if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n') ||
          (codepoint == 0x00A0) || (codepoint == 0x2009))
        endLine = i;

      if ((textOffsetX + glyphWidth) > rec.width) {
        endLine = (endLine < 1) ? i : endLine;
        if (i == endLine)
          endLine -= codepointByteCount;
        if ((startLine + codepointByteCount) == endLine)
          endLine = (i - codepointByteCount);

        state = !state;
      } else if ((i + 1) == length) {
        endLine = i;
        state = !state;
      } else if (codepoint == '\n')
        state = !state;

      if (state == DRAW_STATE) {
        textOffsetX = 0;
        i = startLine;
        glyphWidth = 0;

        // Save character position when we switch states
        int tmp = lastk;
        lastk = k - 1;
        k = tmp;
      }
    } else {
      if (codepoint == '\n') {
        if (!wordWrap) {
          textOffsetY +=
              (font.baseSize + (float)font.baseSize / 2) * scaleFactor;
          textOffsetX = 0;
        }
      } else {
        if (!wordWrap && ((textOffsetX + glyphWidth) > rec.width)) {
          textOffsetY +=
              (font.baseSize + (float)font.baseSize / 2) * scaleFactor;
          textOffsetX = 0;
        }

        // When text overflows rectangle height limit, just stop drawing
        if ((textOffsetY + font.baseSize * scaleFactor) > rec.height)
          break;

        // Draw selection background
        bool isGlyphSelected = false;
        if ((selectStart >= 0) && (k >= selectStart) &&
            (k < (selectStart + selectLength))) {
          DrawRectangleRec((Rectangle){rec.x + textOffsetX - 1,
                                       rec.y + textOffsetY, glyphWidth,
                                       (float)font.baseSize * scaleFactor},
                           selectBackTint);
          isGlyphSelected = true;
        }

        // Draw current character glyph
        if ((codepoint != ' ') && (codepoint != '\t')) {
          DrawTextCodepoint(font, codepoint,
                            (Vector2){rec.x + textOffsetX, rec.y + textOffsetY},
                            fontSize, isGlyphSelected ? selectTint : tint);
        }
      }

      if (wordWrap && (i == endLine)) {
        textOffsetY += (font.baseSize + (float)font.baseSize / 2) * scaleFactor;
        textOffsetX = 0;
        startLine = endLine;
        endLine = -1;
        glyphWidth = 0;
        selectStart += lastk - k;
        k = lastk;

        state = !state;
      }
    }

    if ((textOffsetX != 0) || (codepoint != ' '))
      textOffsetX += glyphWidth; // avoid leading spaces
  }
}
// Draw text using font inside rectangle limits
inline void DrawTextBoxed(Font font, const char *text, Rectangle rec,
                          float fontSize, float spacing, bool wordWrap,
                          Color tint) {
  DrawTextBoxedSelectable(font, text, rec, fontSize, spacing, wordWrap, tint, 0,
                          0, WHITE, WHITE);
}
} // namespace Vendor::raylib

// Provides functions for clay.
namespace Vendor::clay {
inline float GetEstimatedContentHeight(Font font, const char *paragraphs[],
                                       int totalParagraphs, float allowedWidth,
                                       float fontSize,
                                       float lineSpacingFactor) {
  if (totalParagraphs <= 0 || allowedWidth <= 0)
    return 0.0f;

  float totalHeight = 0.0f;

  // Fetch line metrics
  float baseLineHeight = (float)font.baseSize;
  if (fontSize != baseLineHeight) {
    baseLineHeight = fontSize;
  }
  float verticalLineStep = baseLineHeight * lineSpacingFactor;

  float dynamicCharSpacing = (fontSize / (float)font.baseSize);
  if (dynamicCharSpacing < 1.0f)
    dynamicCharSpacing = 1.0f;

  // Pre-calculate width of a standard blank layout space using Raylib
  float spaceWidth = MeasureTextEx(font, " ", fontSize, dynamicCharSpacing).x;

  for (int p = 0; p < totalParagraphs; ++p) {
    std::string_view paragraph = paragraphs[p];

    if (paragraph.empty()) {
      totalHeight += verticalLineStep;
      continue;
    }

    float currentLineWidth = 0.0f;
    int linesInParagraph = 1;

    size_t start = 0;
    size_t length = paragraph.length();

    // High-performance pointer scanning loop (Zero dynamic memory allocations)
    while (start < length) {
      // 0. Count the newlines
      if (paragraph[start] == '\n') {
        linesInParagraph++;
        currentLineWidth = 0.0f;
        start++;
        continue;
      }

      // 1. Skip over any standard spaces
      while (start < length && (paragraph[start] == ' ')) {
        start++;
      }
      if (start >= length)
        break;
      if (paragraph[start] == '\n')
        continue; // keep searching (this will fix it)

      // 2. Find the bound boundary edge of the current word
      size_t end = start;
      while (end < length && paragraph[end] != ' ' && paragraph[end] != '\n') {
        end++;
      }

      // 3. Extract the word slice as a view
      std::string_view word = paragraph.substr(start, end - start);
      start = end; // advance iterator position

      // Convert string_view momentarily to a null-terminated string context
      // *safely* for Raylib DrawTextBoxed handles strings, MeasureTextEx needs
      // a clean buffer. Rather than allocating string vectors, we pass small
      // buffers or use text metrics. Raylib's text measurement requires a null
      // terminator. We can temporarily measure it safely:
      char wordBuffer[256];
      size_t copyLen = (word.length() < 255) ? word.length() : 255;
      char *targetPtr = wordBuffer;

      // Optional: If a single word is insanely massive, allocate safe temporary
      // buffer stack room (Still bypasses global heap overhead completely!)
      std::vector<char> largeWordBuffer;
      if (word.length() >= 255) {
        largeWordBuffer.resize(word.length() + 1);
        targetPtr = largeWordBuffer.data();
        copyLen = word.length();
      }

      std::copy(word.begin(), word.begin() + copyLen, targetPtr);
      targetPtr[copyLen] = '\0';

      // Measure word bounds crisp against Raylib specs
      float wordWidth =
          MeasureTextEx(font, targetPtr, fontSize, dynamicCharSpacing).x;

      if (currentLineWidth == 0.0f) {
        currentLineWidth = wordWidth;
      } else {
        // If appending the word exceeds our right side container boundary, drop
        // down a line box
        if (currentLineWidth + spaceWidth + wordWidth > allowedWidth) {
          linesInParagraph++;
          currentLineWidth = wordWidth;
        } else {
          currentLineWidth += spaceWidth + wordWidth;
        }
      }
    }

    totalHeight += ((float)linesInParagraph * verticalLineStep);

    // Append a half-step gap padding between paragraphs if we aren't handling
    // the last element block
    if (p < totalParagraphs - 1) {
      totalHeight += (verticalLineStep * 0.5f);
    }
  }

  return totalHeight;
  // return 650.0f; // temporary
}

#include <cmath>
// This is the function that is fully Ai.
// This is just for fun.
// Status : Has not been tested.
// HIGH-PERFORMANCE ENGINE ROUTINE: Zero allocations, direct pointer stepping
inline float ComputeDynamicTextHeight(Font font, const char *paragraphs[],
                                      int totalParagraphs, float allowedWidth,
                                      float fontSize, float lineSpacingFactor) {
  if (totalParagraphs <= 0 || allowedWidth <= 0)
    return 0.0f;

  float totalHeight = 0.0f;

  // Extract base font scaling values directly
  float baseLineHeight = (font.baseSize > 0) ? (float)font.baseSize : fontSize;
  if (fontSize != baseLineHeight) {
    baseLineHeight = fontSize;
  }
  float verticalLineStep = baseLineHeight * lineSpacingFactor;

  // Direct cache register lookup for space spacing parameters
  float spaceWidth = MeasureTextEx(font, " ", fontSize, 1.0f).x;

  for (int p = 0; p < totalParagraphs; ++p) {
    const char *run = paragraphs[p];
    if (!run || *run == '\0') {
      totalHeight += verticalLineStep;
      continue;
    }

    float currentLineWidth = 0.0f;
    int linesInParagraph = 1;

    // Pointer-scanning tokenizer loop
    while (*run != '\0') {
      // 1. Skip spaces and clean up embedded newlines
      while (*run == ' ' || *run == '\n') {
        run++;
      }
      if (*run == '\0')
        break;

      // 2. Identify word lengths via pointer subtraction
      const char *wordStart = run;
      while (*run != '\0' && *run != ' ' && *run != '\n') {
        run++;
      }
      int wordLen = static_cast<int>(run - wordStart);

      // 3. Fast Zero-Copy Stack Slicing
      // We isolate the word directly out of memory without modifying the root
      // string
      char wordScratchpad[128];
      int copyLen = (wordLen < 127) ? wordLen : 127;

      // Unrolled hardware layout memory block copy
      for (int i = 0; i < copyLen; ++i) {
        wordScratchpad[i] = wordStart[i];
      }
      wordScratchpad[copyLen] = '\0';

      // Measure word boundary width using Raylib metrics
      float wordWidth = MeasureTextEx(font, wordScratchpad, fontSize, 1.0f).x;

      if (currentLineWidth == 0.0f) {
        currentLineWidth = wordWidth;
      } else {
        // If appending this layout block passes the screen box edge, break down
        // a row
        if (currentLineWidth + spaceWidth + wordWidth > allowedWidth) {
          linesInParagraph++;
          currentLineWidth = wordWidth;
        } else {
          currentLineWidth += spaceWidth + wordWidth;
        }
      }
    }

    totalHeight += (static_cast<float>(linesInParagraph) * verticalLineStep);

    // Append section spacing transitions
    if (p < totalParagraphs - 1) {
      totalHeight += (verticalLineStep * 0.5f);
    }
  }

  return totalHeight;
}
} // namespace Vendor::clay

// ChessMenu for `menu.h`, specifically the options
namespace ChessMenu::logic {
inline void UpdateMenuElement(
    int width, int height, Vector2 mouse,
    int &currentScreenMode, // you can't pass `ChessMenu::CHESSY_UTIL_MENU_MODE`
                            // here since that will result in a compiler panic
                            // about non-const lvalue param reference. Since
                            // anyways the enum members convert to `int`, pass
                            // it like so
    enum ChessUI::CHESSY_UI_MENU_MODE &currentMenuMode) {
  // Mirror the layout metrics utilized inside the DrawStartMenu rendering
  // canvas
  int boardSize = height - 130;
  int panelX = 20 + boardSize + 20;
  int panelW = width - panelX - 20;
  int buttonX = panelX + 20;
  int buttonW = panelW - 40;
  int buttonH = (boardSize - (15 * 5)) / 4;

  // Compute the explicit bounding frame for Button 3 (Daily puzzles)
  int b3Y = (90 + 15) + (buttonH + 15) * 2;
  Rectangle b3Bounds = {(float)buttonX, (float)b3Y, (float)buttonW,
                        (float)buttonH};

  // Compute the explicit bounding frame for Button 4 (Self Practice Card)
  int b4Y = (90 + 15) + (buttonH + 15) * 3;
  Rectangle b4Bounds = {(float)buttonX, (float)b4Y, (float)buttonW,
                        (float)buttonH};

  // Collision logic
  // Button number 4
  if (CheckCollisionPointRec(mouse, b4Bounds)) {
    std::cout << "[ROUTE] : User triggered Menu Card 4 -> Transition to "
                 "Self Practice Mode\n";
    currentScreenMode =
        ChessMode::CHESSY_MODE_PLAYCF; // Slide application into gameplay loop
    currentMenuMode = ChessUI::CHESSY_MODE_PRACTICE;
  }
  // Button number 3
  if (CheckCollisionPointRec(mouse, b3Bounds)) {
    std::cout
        << "[ROUTE] : User triggered Menu Card 3 -> Transition to Puzzles\n";
    currentScreenMode = ChessMode::CHESSY_MODE_PLAYCF;
    currentMenuMode = ChessUI::CHESSY_MODE_PUZZLE;
  }

  // Add more elements below
}

inline void
UpdateMenuHeader(int width, int height, Vector2 mouse, int &currentScreenMode,
                 enum ChessUI::CHESSY_UI_MENU_MODE &currentMenuMode) {
  // Initialize shit.
  static bool hasClicked[5] = {
      false, false, false, false,
      false}; // This is for when we want to know if the user has
              // clicked the header element say twice useful for
              // making double click == previous assignments.
              // Since you can click to navigate, double click ==
              // teleport to home page
              // BTW, i don't wanna include shit and make the
              // binary larger, so C time.

  // Mirror the exact dimensions used in DrawStartMenu

  Rectangle navAbout = {35.0f, 28.0f, 65.0f, 30.0f};
  Rectangle navHelp = {145.0f, 28.0f, 55.0f, 30.0f};
  Rectangle navNews = {235.0f, 28.0f, 55.0f, 30.0f};
  Rectangle navOpeningLib = {325.0f, 28.0f, 175.0f, 30.0f};
  Rectangle navImpExport = {535.0f, 28.0f, 125.0f, 30.0f};

  if (CheckCollisionPointRec(mouse, navAbout)) {
    if (!hasClicked[0])
      std::cout
          << "[ROUTE] : MENU -- User triggered switch to the `About` page "
             "in the header -> Transitioning to `About` page menu...\n";
    else
      std::cout << "[ROUTE] : MENU -- User triggered switch to `Home` page via "
                   "double click in the header -> Tranisitioning to `Home` "
                   "page menu...\n";

    currentScreenMode = ChessMode::CHESSY_MODE_NORMAL;

    if (!hasClicked[0])
      currentMenuMode = ChessUI::CHESSY_MODE_ABOUT;
    else
      currentMenuMode = ChessUI::CHESSY_MODE_HOME;
    if (!hasClicked[0])
      hasClicked[0] =
          true; // 0 is for this. yes i know. this is dangerous.
                // will implement a class like `Colorscheme::operator[]` method
                // to handle this safely (dictionary lookup)
    else
      hasClicked[0] = false;

    // This will be immediately true and thus...
    // THE RESET
    if (hasClicked[0])
      for (SmolInt::nibble_t i = 0; i < 3; ++i)
        hasClicked[i + 1] = false;
  }
  if (CheckCollisionPointRec(mouse, navHelp)) {
    if (!hasClicked[1])
      std::cout << "[ROUTE] : MENU -- User triggered switch to the `Help` page "
                   "in the header -> Transitioning to `Help` page menu...\n";
    else
      std::cout << "[ROUTE] : MENU -- User triggered switch to `Home` page via "
                   "double click in the header -> Tranisitioning to `Home` "
                   "page menu...\n";

    currentScreenMode = ChessMode::CHESSY_MODE_NORMAL;

    if (!hasClicked[1])
      currentMenuMode = ChessUI::CHESSY_MODE_HELP;
    else
      currentMenuMode = ChessUI::CHESSY_MODE_HOME;
    if (!hasClicked[1])
      hasClicked[1] =
          true; // 1 is for this. yes i know. this is dangerous.
                // will implement a class like `Colorscheme::operator[]` method
                // to handle this safely (dictionary lookup)
    else
      hasClicked[1] = false;

    // This will be immediately true and thus...
    // THE RESET
    if (hasClicked[1]) {
      hasClicked[0] = false; // previous
      for (SmolInt::nibble_t i = 0; i < 2; ++i)
        hasClicked[i + 2] = false;
    }
  }
  if (CheckCollisionPointRec(mouse, navNews)) {
  }
  if (CheckCollisionPointRec(mouse, navOpeningLib)) {
  }
  if (CheckCollisionPointRec(mouse, navImpExport)) {
  }
}
} // namespace ChessMenu::logic

#endif
