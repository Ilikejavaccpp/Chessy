#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h> // for sprintf
#include <stdlib.h>
#include <string.h>

#include "../endec_board.h"

/* =========================================================================
 * 1. HIGH-PERFORMANCE STATIC LOOKUPS AND RAYCASTERS (No Allocations)
 * ========================================================================= */

static inline void experimental__reset_coord_brd(uint8_t *board_64) {
  memset(board_64, 0, 64); // fresh wipe
}

static inline void experimental__reset_coord_brd_startpos(uint8_t *board_64) {
  if (!board_64)
    return;
  memcpy(board_64,
         (uint8_t[64]){
             4,  2, 3, 5,  6,  3, 2, 4, // black home row
             1,  1, 1, 1,  1,  1, 1, 1, // black pawns

             0,  0, 0, 0,  0,  0, 0, 0, // empty
             0,  0, 0, 0,  0,  0, 0, 0, // empty

             0,  0, 0, 0,  0,  0, 0, 0, // empty
             0,  0, 0, 0,  0,  0, 0, 0, // empty

             7,  7, 7, 7,  7,  7, 7, 7, // white pawns
             10, 8, 9, 11, 12, 9, 8, 10 // white home row
         },
         64);
}

static inline void
experimental__reset_coord_brd_brd(uint8_t *board_64,
                                  const uint8_t *custom_brd) {
  if (board_64 && custom_brd)
    memcpy(board_64, custom_brd, 64);
}

static inline bool
experimental__check_is_brd_empty(uint8_t *_Nonnull board_64) {
  // A clean reference block of 64 zeroes
  static const uint8_t empty_reference[64] = {0};

  // memcmp returns 0 if all 64 bytes match the reference perfectly
  return memcmp(board_64, empty_reference, 64) == 0;
}

static const char experimental__intToFen_lookup_table[] = {
    ' ',                          // EMPTY = 0
    'p', 'n', 'b', 'r', 'q', 'k', // Black pieces = 1-6
    'P', 'N', 'B', 'R', 'Q', 'K'  // White pieces = 7-12
};

static inline bool is_path_clear(const uint8_t *board, int r1, int c1, int r2,
                                 int c2) {
  int dr = (r2 > r1) ? 1 : ((r2 < r1) ? -1 : 0);
  int dc = (c2 > c1) ? 1 : ((c2 < c1) ? -1 : 0);
  int r = r1 + dr;
  int c = c1 + dc;
  while (r != r2 || c != c2) {
    if (board[r * 8 + c] != 0)
      return false;
    r += dr;
    c += dc;
  }
  return true;
}

static inline int find_origin_square(const uint8_t *board, uint8_t piece_id,
                                     int to_r, int to_c, int req_file,
                                     int req_rank, bool is_capture) {
  bool is_white = (piece_id >= 7);
  uint8_t raw_type = is_white ? (piece_id - 6) : piece_id;

  for (int r = 0; r < 8; ++r) {
    if (req_rank != -1 && r != req_rank)
      continue;
    for (int c = 0; c < 8; ++c) {
      if (req_file != -1 && c != req_file)
        continue;

      int idx = r * 8 + c;
      if (board[idx] != piece_id)
        continue;

      uint8_t target_p = board[to_r * 8 + to_c];
      if (target_p != 0 && ((target_p >= 7) == is_white))
        continue;

      int dr = abs(to_r - r);
      int dc = abs(to_c - c);

      switch (raw_type) {
      case 1: // Pawn
        if (is_white) {
          if (!is_capture && dc == 0) {
            if (dr == 1 && target_p == 0)
              return idx;
            if (dr == 2 && r == 6 && target_p == 0 && board[5 * 8 + c] == 0)
              return idx;
          } else if (is_capture && dr == 1 && dc == 1) {
            return idx;
          }
        } else { // Black Pawn
          if (!is_capture && dc == 0) {
            if (dr == 1 && target_p == 0)
              return idx;
            if (dr == 2 && r == 1 && target_p == 0 && board[2 * 8 + c] == 0)
              return idx;
          } else if (is_capture && dr == 1 && dc == 1) {
            return idx;
          }
        }
        break;

      case 2: // Knight
        if ((dr == 1 && dc == 2) || (dr == 2 && dc == 1))
          return idx;
        break;

      case 3: // Bishop
        if (dr == dc && is_path_clear(board, r, c, to_r, to_c))
          return idx;
        break;

      case 4: // Rook
        if ((dr == 0 || dc == 0) && is_path_clear(board, r, c, to_r, to_c))
          return idx;
        break;

      case 5: // Queen
        if ((dr == dc || dr == 0 || dc == 0) &&
            is_path_clear(board, r, c, to_r, to_c))
          return idx;
        break;

      case 6: // King
        if (dr <= 1 && dc <= 1)
          return idx;
        break;
      }
    }
  }
  return -1;
}

/* =========================================================================
 * 2. CONVERT PGN TO UCI (Contiguous Arena Stream)
 * ========================================================================= */

void experimental_convert_pgnToUCI(int8_t *restrict *out_uci,
                                   int8_t *restrict pgn_lines, const char *mode,
                                   const uint8_t *start_pos_board) {
  if (!out_uci || !pgn_lines)
    return;

  // Fast contiguous allocation of target buffer (Batch Arena)
  if (*out_uci == NULL) {
    *out_uci = malloc(CHESSY_BACKEND_C_CORE_FILEIO_FLAG__VBATCH_MAX * 2);
    if (!*out_uci)
      return;
  }

  // --- PIPELINE GL-STYLE PRE-COMPUTE STEP ---
  // Extract configuration states outside the loop to keep parsing
  // branchless/cheap
  const char __nl_mode = (char)*CHESSY_BACKEND__DEFAULT_MODE_NL;
  const bool opt_nl_ns =
      (__nl_mode == (char)*CHESSY_BACKEND__MODE_NL_NS); // No Space mode
  const bool opt_nl_ws =
      (__nl_mode == (char)*CHESSY_BACKEND__MODE_NL_WS); // With Space mode
  const bool mode_oldschool = mode ? (strchr(mode, 'o') != NULL) : false;

  uint8_t local_board[64];
  const char *p = (const char *)pgn_lines;
  int out_i = 0;
  bool is_white_turn = true;

  // Fast layout initialization
  if (start_pos_board &&
      !experimental__check_is_brd_empty((uint8_t *)start_pos_board)) {
    experimental__reset_coord_brd_brd(local_board, start_pos_board);
  } else {
    experimental__reset_coord_brd_startpos(local_board);
  }

  while (*p != '\0') {

    // ----------------------------------------------------
    // [IF for 1] - Pipeline Step 1: Whitespace & Newlines
    // ----------------------------------------------------
    if (isspace((unsigned char)*p)) {
      if (*p == '\n') {
        if (out_i > 0 && (*out_uci)[out_i - 1] == ' ') {
          if (opt_nl_ns) {
            (*out_uci)[out_i - 1] = '\n'; // Overwrite trailing space
          } else if (opt_nl_ws) {
            (*out_uci)[out_i++] = '\n'; // Keep space and append
          }
        } else {
          (*out_uci)[out_i++] = '\n';
        }

        // Reset board layout for the next line in the batch
        if (start_pos_board &&
            !experimental__check_is_brd_empty((uint8_t *)start_pos_board)) {
          experimental__reset_coord_brd_brd(local_board, start_pos_board);
        } else {
          experimental__reset_coord_brd_startpos(local_board);
        }
        is_white_turn = true;
      }
      p++;
      continue;
    }

    // ----------------------------------------------------
    // Pipeline Step 2: Skip Move Indicators (e.g. "1.")
    // ----------------------------------------------------
    if (isdigit((unsigned char)*p)) {
      while (*p && !isspace((unsigned char)*p) && *p != '.')
        p++;
      while (*p == '.')
        p++;
      continue;
    }

    // ----------------------------------------------------
    // Pipeline Step 3: Zero-Allocation Arena Pointer Mapping
    // ----------------------------------------------------
    // Instead of allocating a token string, map a local sliding buffer
    char token[16];
    int t_len = 0;
    while (*p && !isspace((unsigned char)*p) && t_len < 15) {
      token[t_len++] = *p++;
    }
    token[t_len] = '\0';

    // Fast-strip check and evaluation decorators
    while (t_len > 0 && (token[t_len - 1] == '+' || token[t_len - 1] == '#' ||
                         token[t_len - 1] == '?' || token[t_len - 1] == '!')) {
      token[--t_len] = '\0';
    }

    if (t_len == 0)
      continue;

    int from_r = -1, from_c = -1, to_r = -1, to_c = -1;
    char promo = '\0';

    // ----------------------------------------------------
    // Pipeline Step 4: Branchless-first Parsing
    // ----------------------------------------------------
    // Optimize string matches: Castling options always start with 'O' or '0'
    if (token[0] == 'O' || token[0] == '0') {
      bool is_queenside = (t_len > 3); // O-O-O vs O-O
      if (is_white_turn) {
        from_r = 7;
        from_c = 4;
        to_r = 7;
        to_c = is_queenside ? 2 : 6;
        local_board[60] = 0;
        local_board[to_r * 8 + to_c] = 12;
        if (is_queenside) {
          local_board[56] = 0;
          local_board[59] = 10;
        } else {
          local_board[63] = 0;
          local_board[61] = 10;
        }
      } else {
        from_r = 0;
        from_c = 4;
        to_r = 0;
        to_c = is_queenside ? 2 : 6;
        local_board[4] = 0;
        local_board[to_r * 8 + to_c] = 6;
        if (is_queenside) {
          local_board[0] = 0;
          local_board[3] = 4;
        } else {
          local_board[7] = 0;
          local_board[5] = 4;
        }
      }
      (*out_uci)[out_i++] = 'a' + from_c;
      (*out_uci)[out_i++] = '8' - from_r;
      (*out_uci)[out_i++] = 'a' + to_c;
      (*out_uci)[out_i++] = '8' - to_r;
      (*out_uci)[out_i++] = ' ';
    } else {
      // Check promotion directly
      char *eq = strchr(token, '=');
      if (eq) {
        promo = tolower((unsigned char)*(eq + 1));
        *eq = '\0';
        t_len = strlen(token);
      }

      // Map Piece Identity without branching sequences
      uint8_t p_type = 1; // Pawn
      int parse_idx = 0;
      switch (token[0]) {
      case 'N':
        p_type = 2;
        parse_idx = 1;
        break;
      case 'B':
        p_type = 3;
        parse_idx = 1;
        break;
      case 'R':
        p_type = 4;
        parse_idx = 1;
        break;
      case 'Q':
        p_type = 5;
        parse_idx = 1;
        break;
      case 'K':
        p_type = 6;
        parse_idx = 1;
        break;
      }

      uint8_t piece_id = is_white_turn ? (p_type + 6) : p_type;

      to_c = token[t_len - 2] - 'a';
      to_r = '8' - token[t_len - 1];

      int req_file = -1, req_rank = -1;
      bool is_capture = false;

      // Parse qualifiers in one fast sliding pass
      for (int k = parse_idx; k < t_len - 2; ++k) {
        if (token[k] == 'x') {
          is_capture = true;
        } else if (token[k] >= 'a' && token[k] <= 'h') {
          req_file = token[k] - 'a';
        } else if (token[k] >= '1' && token[k] <= '8') {
          req_rank = '8' - token[k];
        }
      }

      int origin = find_origin_square(local_board, piece_id, to_r, to_c,
                                      req_file, req_rank, is_capture);
      if (origin != -1) {
        from_r = origin / 8;
        from_c = origin % 8;

        // En Passant detection
        if (p_type == 1 && to_c != from_c &&
            local_board[to_r * 8 + to_c] == 0) {
          local_board[from_r * 8 + to_c] = 0;
        }

        local_board[origin] = 0;
        local_board[to_r * 8 + to_c] =
            promo ? (is_white_turn ? (promo == 'q'   ? 11
                                      : promo == 'r' ? 10
                                      : promo == 'b' ? 9
                                                     : 8)
                                   : (promo == 'q'   ? 5
                                      : promo == 'r' ? 4
                                      : promo == 'b' ? 3
                                                     : 2))
                  : piece_id;

        (*out_uci)[out_i++] = 'a' + from_c;
        (*out_uci)[out_i++] = '8' - from_r;
        (*out_uci)[out_i++] = 'a' + to_c;
        (*out_uci)[out_i++] = '8' - to_r;
        if (promo) {
          (*out_uci)[out_i++] = promo;
        }
        (*out_uci)[out_i++] = ' ';
      }
    }
    is_white_turn = !is_white_turn;
  }

  // Finalize state
  if (out_i > 0 && (*out_uci)[out_i - 1] == ' ') {
    (*out_uci)[opt_nl_ns ? (out_i - 1) : out_i] = '\0';
  } else {
    (*out_uci)[out_i] = '\0';
  }
}

/* =========================================================================
 * 3. CONVERT UCI TO PGN (Contiguous Arena Stream)
 * ========================================================================= */

void experimental_convert_uciToPGN(int8_t *restrict *out_pgn,
                                   int8_t *restrict uci_lines,
                                   const uint8_t *start_pos_board) {
  if (!out_pgn || !uci_lines)
    return;

  if (*out_pgn == NULL) {
    *out_pgn = malloc(CHESSY_BACKEND_C_CORE_FILEIO_FLAG__VBATCH_MAX * 2);
    if (!*out_pgn)
      return;
  }

  // --- PIPELINE GL-STYLE PRE-COMPUTE STEP ---
  const char __nl_mode = (char)*CHESSY_BACKEND__DEFAULT_MODE_NL;
  const bool opt_nl_ns = (__nl_mode == (char)*CHESSY_BACKEND__MODE_NL_NS);

  uint8_t local_board[64];
  const char *p = (const char *)uci_lines;
  int out_i = 0;
  int move_number = 1;
  bool is_white_turn = true;

  if (start_pos_board &&
      !experimental__check_is_brd_empty((uint8_t *)start_pos_board)) {
    experimental__reset_coord_brd_brd(local_board, start_pos_board);
  } else {
    experimental__reset_coord_brd_startpos(local_board);
  }

  while (*p != '\0') {

    // ----------------------------------------------------
    // [IF for 1] - Pipeline Step 1: Newline Formatting
    // ----------------------------------------------------
    if (isspace((unsigned char)*p)) {
      if (*p == '\n') {
        if (out_i > 0 && (*out_pgn)[out_i - 1] == ' ') {
          if (opt_nl_ns) {
            (*out_pgn)[out_i - 1] = '\n';
          } else {
            (*out_pgn)[out_i++] = '\n';
          }
        } else {
          (*out_pgn)[out_i++] = '\n';
        }

        // Reset board layout for the next batch line
        if (start_pos_board &&
            !experimental__check_is_brd_empty((uint8_t *)start_pos_board)) {
          experimental__reset_coord_brd_brd(local_board, start_pos_board);
        } else {
          experimental__reset_coord_brd_startpos(local_board);
        }
        is_white_turn = true;
        move_number = 1;
      }
      p++;
      continue;
    }

    // ----------------------------------------------------
    // Pipeline Step 2: Token Extraction (Arena style pointer matching)
    // ----------------------------------------------------
    char token[16];
    int t_len = 0;
    while (*p && !isspace((unsigned char)*p) && t_len < 15) {
      token[t_len++] = *p++;
    }
    token[t_len] = '\0';

    if (t_len < 4)
      continue;

    int from_c = token[0] - 'a';
    int from_r = '8' - token[1];
    int to_c = token[2] - 'a';
    int to_r = '8' - token[3];
    char promo = (t_len > 4) ? token[4] : '\0';

    uint8_t piece = local_board[from_r * 8 + from_c];
    uint8_t p_type = (piece >= 7) ? (piece - 6) : piece;
    bool is_capture = (local_board[to_r * 8 + to_c] != 0);

    if (is_white_turn) {
      out_i += sprintf((char *)*out_pgn + out_i, "%d. ", move_number);
    }

    // ----------------------------------------------------
    // Pipeline Step 3: Castling Generation
    // ----------------------------------------------------
    if (p_type == 6 && abs(to_c - from_c) == 2) {
      if (to_c == 6) {
        out_i += sprintf((char *)*out_pgn + out_i, "O-O ");
        local_board[from_r * 8 + 5] = local_board[from_r * 8 + 7];
        local_board[from_r * 8 + 7] = 0;
      } else {
        out_i += sprintf((char *)*out_pgn + out_i, "O-O-O ");
        local_board[from_r * 8 + 3] = local_board[from_r * 8 + 0];
        local_board[from_r * 8 + 0] = 0;
      }
      local_board[to_r * 8 + to_c] = piece;
      local_board[from_r * 8 + from_c] = 0;
    }
    // ----------------------------------------------------
    // Pipeline Step 4: Normal Move Generation
    // ----------------------------------------------------
    else {
      static const char piece_symbols[] = {' ', ' ', 'N', 'B', 'R', 'Q', 'K'};
      char prefix = piece_symbols[p_type];

      if (prefix != ' ') {
        (*out_pgn)[out_i++] = prefix;

        // Disambiguation calculation
        bool need_file = false, need_rank = false;
        for (int r = 0; r < 8; ++r) {
          for (int c = 0; c < 8; ++c) {
            if (r == from_r && c == from_c)
              continue;
            if (local_board[r * 8 + c] == piece) {
              int dr = abs(to_r - r);
              int dc = abs(to_c - c);
              bool can_attack = false;
              if (p_type == 2 && ((dr == 1 && dc == 2) || (dr == 2 && dc == 1)))
                can_attack = true;
              if (p_type == 3 && dr == dc &&
                  is_path_clear(local_board, r, c, to_r, to_c))
                can_attack = true;
              if (p_type == 4 && (dr == 0 || dc == 0) &&
                  is_path_clear(local_board, r, c, to_r, to_c))
                can_attack = true;
              if (p_type == 5 && (dr == dc || dr == 0 || dc == 0) &&
                  is_path_clear(local_board, r, c, to_r, to_c))
                can_attack = true;

              if (can_attack) {
                if (c != from_c)
                  need_file = true;
                else
                  need_rank = true;
              }
            }
          }
        }
        if (need_file)
          (*out_pgn)[out_i++] = 'a' + from_c;
        if (need_rank)
          (*out_pgn)[out_i++] = '8' - from_r;
      }

      if (p_type == 1 && to_c != from_c && !is_capture) {
        is_capture = true;
        local_board[from_r * 8 + to_c] = 0; // EP removal
      }

      if (is_capture) {
        if (p_type == 1) {
          (*out_pgn)[out_i++] = 'a' + from_c;
        }
        (*out_pgn)[out_i++] = 'x';
      }

      (*out_pgn)[out_i++] = 'a' + to_c;
      (*out_pgn)[out_i++] = '8' - to_r;

      if (promo) {
        (*out_pgn)[out_i++] = '=';
        (*out_pgn)[out_i++] = toupper((unsigned char)promo);
      }

      (*out_pgn)[out_i++] = ' ';

      local_board[from_r * 8 + from_c] = 0;
      local_board[to_r * 8 + to_c] =
          promo ? (is_white_turn ? (promo == 'q'   ? 11
                                    : promo == 'r' ? 10
                                    : promo == 'b' ? 9
                                                   : 8)
                                 : (promo == 'q'   ? 5
                                    : promo == 'r' ? 4
                                    : promo == 'b' ? 3
                                                   : 2))
                : piece;
    }

    if (!is_white_turn) {
      move_number++;
    }
    is_white_turn = !is_white_turn;
  }

  // Final clean termination
  if (out_i > 0 && (*out_pgn)[out_i - 1] == ' ') {
    (*out_pgn)[opt_nl_ns ? (out_i - 1) : out_i] = '\0';
  } else {
    (*out_pgn)[out_i] = '\0';
  }
}
