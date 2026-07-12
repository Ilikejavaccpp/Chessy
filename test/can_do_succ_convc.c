// NOTE: this file is ai generated, the functions are ^C^V from this codebase
// (via ai, since again... speed and also they are written by me not ai, so that
// again means i have to send the individual files or code fences) but the main
// and other so called "non-fileio" (i.e. not from
// `include/backend/file_io.(h/c)`) but with some includes to fasten up some
// stuff. NOTE: the functions are now included.

#include "backend/endec_board.h"
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
                   "nx");

  printf("Was just a bug.\n");

  return 0;
}
