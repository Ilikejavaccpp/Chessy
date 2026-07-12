// NOTE: this file is ai generated, the functions are ^C^V from this codebase
// (via ai, since again... speed and also they are written by me not ai, so that
// again means i have to send the individual files or code fences) but the main
// and other so called "non-fileio" (i.e. not from
// `include/backend/file_io.(h/c)`) but with some includes to fasten up some
// stuff. NOTE: the functions are now included.

// --F-I-X-M-E-'s [DONE]
// - truncation of the first character of variable `uci_positions` index 21
// (actual C index `uci_positions[20]`), was because of wrong boundary limiter
//
// NOTE:
// - the truncation happens with ANY character not just an `e`
// May be because of poor writing (encoding) and/or decoding (rm'ing the
// test bin file auto generated doesn't work, HELP ME), it was because
// of my poor eye. NO NEED NOW

// #include "backend/macros.h"

#include "backend/file_io.h"
#include "backend/smolInt.h"

#include "backend/endec_board.h"

#include "backend/smolInt.c" // unity build

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *restrict uci_positions =
    "e2e4\n"           // The Open Game / 1. e4
    "d2d4 d7d5 c2c4\n" // The Queens Gambit
    "e2e4 c7c5\n"      // The Sicilian (my friend (alpines defence-WHITE or the
                       // dragon-BLACK) and enemy)
    ;
static char *restrict uci_position[3] = {
    "e2e4\n",           // The Open Game / 1. e4
    "d2d4 d7d5 c2c4\n", // The Queens Gambit
    "e2e4 c7c5\n",      // The Sicilian (my friend (alpines defence-WHITE or the
                        // dragon-BLACK) and enemy)
};

// Safe string payload content checking instead of tracking volatile memory
// segments
// if (out_lines != NULL && strlen(out_lines) > 0 &&
//     strcmp(out_lines, uci_positions) == 0) {
//   printf("[INFO] : It works!! Decoded output: %s\n", out_lines);
// }
// // elif (out_lines != NULL) {
// //   printf("[WARNING] : Encoding is messed up. ;-;\n");
// // }
// elif (out_lines != (char *)uci_positions) {
//   printf("[WARNING] : Decoding is messed up. ;-;\n");
// }
// else {
//   printf("[ERROR] : The encode/decode machine for `engine lines` is messed
//   "
//          "up.\n  Exiting...\n");
//   return 1;
// }

// Buffers allocated safely outside the stack frame to prevent overflows
char ol_vbuf[512] = {0};
unsigned short es_vbuf[3] = {0};

// out_lines needs to be a regular pointer so decodeEngineLines can change where
// it points via malloc
static char *out_lines = NULL;
static unsigned short *eval_scores = es_vbuf;
static unsigned char depth;

int main() {
  printf("[BENCHMARK] : \n");
  printf("[INFO] : Setting up some 3 dummy chess positions with ucis.\n");
  nibble_t i;

  for (initNibbleint(&i); (*(unsigned char *)&(i)) < 3;
       SmolInt_incPre_nibble(&i)) {
    printf("[INFO] : Setting up chess position with UCI: \n  %s",
           uci_position[(*(unsigned char *)&(i))]);
  }

  // Mock standard chess starting array setup (65 elements allocated)
  uint8_t mock_board[65] = {4, 2, 3, 5, 6, 3, 2, 4, 1,  1, 1, 1,  1,  1, 1, 1,
                            0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0,  0,  0, 0, 0,
                            7, 7, 7, 7, 7, 7, 7, 7, 10, 8, 9, 11, 12, 9, 8, 10};
  mock_board[64] = 0; // Essential sentinel terminator for packSmallBatch

  printf("[INFO] : Encoding engine lines + board\n");

  // Clear previous runs
  remove("hardware_engln_board.bin");

  encodeMatrix("hardware_engln_board.bin", mock_board);
  encodeEngineLines("hardware_engln_board.bin", (int8_t *)uci_positions, 1, 25);

  printf("[INFO] : Printing the binary...\n");
  system("cat hardware_engln_board.bin");

  printf("\n[INFO] : Decoding the file (engine lines)...\n");

  unsigned short *eval_scores_ptr = es_vbuf;
  decodeEngineLines("hardware_engln_board.bin",
                    (signed char **restrict)&out_lines, &depth,
                    (unsigned short **restrict)&eval_scores_ptr);

  printf("[INFO] : Verifying the contents (engine lines)... -> WAIT\n");

  // Construct what the expected output string should be
  char expected_total[512] = {0};
  strcat(expected_total, "e2e4\n");
  strcat(expected_total, "d2d4 d7d5 c2c4\n");
  strcat(expected_total, "e2e4 c7c5\n");

  // Print results to see details
  printf("OUTPUT LINES -> %s\n", out_lines ? out_lines : "NULL");
  printf("INPUT LINES  -> %s\n", expected_total);

  // Perform correct string comparison instead of pointer mapping
  if (out_lines != NULL && strlen(out_lines) > 0 &&
      strcmp(out_lines, expected_total) == 0) {
    printf("[INFO] : It works!! Decoded output matches perfectly.\n");
  } else {
    printf("[WARNING] : Decoding values do not match standard inputs. ;-;\n");
  }

  // Clean up heap memory dynamically allocated by decodeEngineLines
  if (out_lines) {
    free(out_lines);
  }

  return 0;
}
