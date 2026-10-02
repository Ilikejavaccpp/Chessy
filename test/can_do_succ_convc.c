// NOTE: this file is ai generated, the functions are ^C^V from this codebase
// (via ai, since again... speed and also they are written by me not ai, so that
// again means i have to send the individual files or code fences) but the main
// and other so called "non-fileio" (i.e. not from
// `include/backend/file_io.(h/c)`) but with some includes to fasten up some
// stuff. NOTE: the functions are now included.

#include "backend/endec_board.h"
#include "backend/macros.h"
#include <stdio.h>

static const char *restrict uci_positions =
    "e2e4\n"           // The Open Game / 1. e4
    "d2d4 d7d5 c2c4\n" // The Queens Gambit
    "e2e4 c7c5\n"      // The Sicilian (my friend (alpines defence-WHITE or the
                       // dragon-BLACK) and enemy)
    ;
static const char *restrict pgn_positions = "1. e4\n"
                                            "1. d4 d5 2. c4\n"
                                            "1. e4 c5\n";

int main() {
  // Declare our dummy placeholder output holder
  char out_uci[CHESSY_BACKEND_C_CORE_FILEIO_FLAG__VBATCH_MAX];

  convert_pgnToUCI((int8_t *restrict *)&out_uci, (signed char *)pgn_positions,
                   CHESSY_BACKEND__MODE_PGN_NOTATION_OLD
                       CHESSY_BACKEND__MODE_PGN_NOTATION_USECAPTURE); // WORKS

  // Test if we can work with the out_uci
  info_log("DECRYPT -- state-log:: UCI variable `out_uci` is \n"
           "char -- %c \n"
           "ascii-- %d",
           out_uci[0], out_uci[0]);

  // Test if "write" operation is possible on the char pointer (mutex string)
  out_uci[1] = 'H';
  out_uci[2] = 'I';
  out_uci[3] = '\n';
  for (uint8_t i = 4; i < 255; ++i) {
    out_uci[i] = 0;
  }

  info_log("WRITE -- state-log:;afterw:: UCI variable `out_uci` is \n"
           "chars (RAW, below) -- \n"
           "%s",
           out_uci);
  info_log("It works! operation `write` works.\n");

  return 0;
}
