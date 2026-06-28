// NOTE: this file is ai generated, the functions are ^C^V from this codebase
// (via ai, since again... speed and also they are written by me not ai, so that
// again means i have to send the individual files or code fences) but the main
// and other so called "non-fileio" (i.e. not from
// `include/backend/file_io.(h/c)`)
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Type aliases matching your architecture environment
typedef uint8_t bite_t;

#define CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX 0x0B
#define SMOL_INT_LIBRARY__NIBBLE_MAXSIZE 15
#define CHESSY_BACKEND__SMALL_BATCH_COUNT_MAX SMOL_INT_LIBRARY__NIBBLE_MAXSIZE

// Core Structural Memory Mappings
struct PackedPair8 {
  uint8_t left : 4;  // 4 bits: range 0 to 15 (Run length count)
  uint8_t right : 4; // 4 bits: range 0 to 15 (Obfuscated piece ID)
};

// ============================================================================
// ORIGINAL ARCHITECTURE DRIVERS (NO REWRITING / MODIFICATIONS)
// ============================================================================

void appendBinary(FILE **restrict __overw_file,
                  const char *restrict file_name) {
  *__overw_file = fopen(file_name, "ab");
}

void writeBinary(FILE **restrict __overw_file, const char *restrict file_name) {
  *__overw_file = fopen(file_name, "wb");
}

void readBinary(FILE **restrict __read_file, const char *restrict file_name) {
  *__read_file = fopen(file_name, "rb");
}

void closeBinary(FILE **restrict __close_file) {
  if (*__close_file) {
    fclose(*__close_file);
    *__close_file = NULL;
  }
}

void testBinary(FILE **restrict __test_file, uint8_t *out_flag) {
  if (!(*__test_file)) {
    *out_flag = 255;
    return;
  }
  *out_flag = 0;
}

// A helper structure to measure exactly what your file driver is writing
long get_file_size(const char *filename) {
  FILE *f = fopen(filename, "rb");
  if (!f)
    return 0;
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fclose(f);
  return size;
}

// Drops dummy PackedPair8 blocks to pad the file out to a fixed chunk size
// boundary
void padToFixedBlockBoundary(const char *filename, long expected_blocks) {
  FILE *f = fopen(filename, "ab");
  if (!f)
    return;

  long current_size = get_file_size(filename);
  long current_blocks = current_size / sizeof(struct PackedPair8);
  long missing_blocks = expected_blocks - current_blocks;

  struct PackedPair8 padding_block = {.left = 0, .right = 0};
  for (long i = 0; i < missing_blocks; i++) {
    fwrite(&padding_block, sizeof(struct PackedPair8), 1, f);
  }
  fclose(f);
}

void packSmallBatch(const char *file_name, uint8_t *restrict batch_64) {
  FILE *o_file = NULL;
  uint8_t error;
  writeBinary(&o_file, file_name);
  testBinary(&o_file, &error);

  if (error != 0)
    return;

  struct PackedPair8 block;

  for (bite_t i = 0; i < 64; ++i) {
    uint8_t pointer = batch_64[i];
    block.left = 1;

    while ((bite_t)(i + 1) < 64 && batch_64[i] == batch_64[i + 1] &&
           block.left < 15 && (i % 8 != 7)) {
      block.left++;
      i++;
    }

    block.right = (pointer ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX) &
                  CHESSY_BACKEND__SMALL_BATCH_COUNT_MAX;

    fwrite(&block, sizeof(struct PackedPair8), 1, o_file);
  }

  closeBinary(&o_file);
}

void unpackSmallBatch(const char *restrict file_name,
                      uint8_t *restrict out_batch_64) {
  FILE *i_file = NULL;
  uint8_t error;
  readBinary(&i_file, file_name);
  testBinary(&i_file, &error);

  if (error != 0)
    return;

  struct PackedPair8 block;
  bite_t i = 0;

  while (fread(&block, sizeof(struct PackedPair8), 1, i_file) == 1) {
    uint8_t count = block.left;
    uint8_t id = (block.right ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX) &
                 CHESSY_BACKEND__SMALL_BATCH_COUNT_MAX;

    for (uint8_t j = 0; j < count; ++j) {
      if (i < 64)
        out_batch_64[i++] = id;
    }
  }

  closeBinary(&i_file);
}

// ============================================================================
// BOARD ENCODING (extras) FUNCTIONS FOR NO OVERWRITE
// ============================================================================
void packAppendSmallBatch(const char *file_name,
                          const uint8_t *restrict batch_64) {
  FILE *o_file = NULL;
  uint8_t error;
  appendBinary(&o_file, file_name);
  testBinary(&o_file, &error);

  if (error != 0)
    return;

  struct PackedPair8 block;

  for (bite_t i = 0; i < 64; ++i) {
    uint8_t pointer = batch_64[i];
    block.left = 1;

    while ((bite_t)(i + 1) < 64 && batch_64[i] == batch_64[i + 1] &&
           block.left < 15 && (i % 8 != 7)) {
      block.left++;
      i++;
    }

    block.right = (pointer ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX) &
                  CHESSY_BACKEND__SMALL_BATCH_COUNT_MAX;

    fwrite(&block, sizeof(struct PackedPair8), 1, o_file);
  }

  closeBinary(&o_file);
}

// ============================================================================
// RANDOM-ACCESS SEEK EXTENSION
// ============================================================================

void unpackSmallBatchIndexed(const char *restrict file_name,
                             uint8_t *restrict out_batch_64,
                             const uint32_t *restrict chunk_jump) {
  FILE *i_file = NULL;
  uint8_t error;
  readBinary(&i_file, file_name);
  testBinary(&i_file, &error);

  if (error != 0 || !out_batch_64) {
    if (i_file)
      closeBinary(&i_file);
    return;
  }

  if (chunk_jump != NULL && *chunk_jump > 0) {
    // Skips whole blocks instantly via hardware file positioning pointers
    long target_offset = (long)(*chunk_jump * 64 * sizeof(struct PackedPair8));
    fseek(i_file, target_offset, SEEK_SET);
  }

  struct PackedPair8 block;
  bite_t i = 0;

  while (fread(&block, sizeof(struct PackedPair8), 1, i_file) == 1) {
    uint8_t count = block.left;
    uint8_t id = (block.right ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX) &
                 CHESSY_BACKEND__SMALL_BATCH_COUNT_MAX;

    for (uint8_t j = 0; j < count; ++j) {
      if (i < 64) {
        out_batch_64[i++] = id;
      }
    }
    if (i >= 64)
      break; // Finished building target block chunk boundary
  }

  closeBinary(&i_file);
}

// ============================================================================
// HARDWARE PERFORMANCE BENCHMARK SUITE
// ============================================================================

int main(void) {
  const char *test_bin = "hardware_speed_test_board.bin";

  // Mock standard chess starting array setup
  // 1-6 --> black pieces ranked ascending (piece value, bishop <= 3.5, >= 3.0)
  // 7-12 -> white pieces ranked ascending (piece value, bishop <= 3.5, >= 3.0)
  // 0   --> EMPTY
  uint8_t mock_board[64] = {
      4, 2, 3, 5, 6, 3, 2, 4, 1, 1, 1, 1, 1,  1,  1,  1,  0,  0,  0,  0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0, 0, 0,
      0, 0, 0, 0, 9, 9, 9, 9, 9, 9, 9, 9, 12, 10, 11, 13, 14, 11, 10, 12};
  uint8_t out_board[64] = {0};

  uint8_t board_open_game[64] = {
      4,  0, 3,  5,  6,  3,  2,  4, // White pieces (Nf3 moved out)
      1,  1, 1,  1,  0,  1,  1,  1, // White pawns (e4 moved out)
      0,  0, 0,  0,  0,  2,  0,  0, // Knight lands on f3 (idx 21)
      0,  0, 0,  0,  1,  0,  0,  0, // White pawn lands on e4 (idx 28)
      0,  0, 0,  0,  9,  0,  0,  0, // Black pawn lands on e5 (idx 36)
      0,  0, 10, 0,  0,  0,  0,  0, // Black knight lands on c6 (idx 42)
      9,  9, 9,  9,  0,  9,  9,  9, // Black pawns (e5 moved out)
      12, 0, 11, 13, 14, 11, 10, 12 // Black pieces (Nc6 moved out)
  };

  uint8_t board_queens_gambit[64] = {4, 2, 3, 5, 6, 3, 2, 4, 1, 1, 0, 0, 1, 1,
                                     1, 1, // White pawns (c4 and d4 moved out)
                                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
                                     0, 0, // White c4 (idx 26) and d4 (idx 27)
                                           // pawns active
                                     0, 0, 0, 9, 0, 0, 0,
                                     0, // Black d5 pawn active (idx 35)
                                     0, 0, 0, 0, 0, 0, 0, 0, 9, 9, 9, 0, 9, 9,
                                     9, 9, // Black pawns (d5 moved out)
                                     12, 10, 11, 13, 14, 11, 10, 12};

  // a dummy, may hold the queen's gambit
  uint8_t mock_board_dumdum[64] = {0};

  printf("[TEST] Beginning hardware serialization validation...\n");

  // 1. Benchmark Encoding
  clock_t start_pack = clock();
  packSmallBatch(test_bin, mock_board); // acts as a clear-write
  padToFixedBlockBoundary(test_bin, 64);
  packAppendSmallBatch(test_bin, board_open_game); // appends board
  padToFixedBlockBoundary(test_bin, 128);
  packAppendSmallBatch(test_bin, board_queens_gambit); // appends board
  padToFixedBlockBoundary(test_bin, 192);
  clock_t end_pack = clock();
  double time_pack = ((double)(end_pack - start_pack)) / CLOCKS_PER_SEC;
  printf("[SPEED] Pack Execution Time: %f seconds\n", time_pack);

  // 2. Benchmark Full Decoding
  unsigned int chunk_jump_2 = 2;
  unsigned int chunk_jump_3 = 3;

  clock_t start_unpack = clock();
  unpackSmallBatch(test_bin, out_board);
  unpackSmallBatchIndexed(test_bin, mock_board_dumdum, &chunk_jump_2);
  unpackSmallBatchIndexed(test_bin, mock_board_dumdum, &chunk_jump_3);
  clock_t end_unpack = clock();
  double time_unpack = ((double)(end_unpack - start_unpack)) / CLOCKS_PER_SEC;
  printf("[SPEED] Unpack Execution Time: %f seconds\n", time_unpack);

  // 3. Verify Array Integrity
  int integrity_check = 1;
  for (int i = 0; i < 64; i++) {
    if (mock_board[i] != out_board[i]) {
      integrity_check = 0;
      break;
    }
  }
  printf("[STATUS] Matrix Validation Integrity: %s\n",
         integrity_check ? "PASSED (100% Match)" : "FAILED");
  printf("[INFO] Board unpacked...\n> board\n  board:\n");
  for (uint8_t i = 0; i < 64; ++i) {
    printf((i % 8 != 7) ? " %d" : " %d\n", out_board[i]);
  }
  printf("\n");

  // 1st real game
  unpackSmallBatchIndexed(test_bin, mock_board_dumdum, &chunk_jump_2);
  for (uint8_t i = 0; i < 64; ++i) {
    printf((i % 8 != 7) ? " %d" : " %d\n", mock_board_dumdum[i]);
  }
  printf("\n");

  // 2nd real game
  unpackSmallBatchIndexed(test_bin, mock_board_dumdum, &chunk_jump_3);
  for (uint8_t i = 0; i < 64; ++i) {
    printf((i % 8 != 7) ? " %d" : " %d\n", mock_board_dumdum[i]);
  }
  printf("\n");

  // 4. Test Indexed Jump-Seeking Extension
  uint32_t target_jump =
      0; // Check index 0 via direct address jump offset pointer
  memset(out_board, 0, 64);

  clock_t start_seek = clock();
  unpackSmallBatchIndexed(test_bin, out_board, &target_jump);
  clock_t end_seek = clock();
  double time_seek = ((double)(end_seek - start_seek)) / CLOCKS_PER_SEC;
  printf("[SPEED] Indexed Jumper Seek Execution Time: %f seconds\n", time_seek);

  // Cleanup disk space
  // OPTIONAL, not here for the data..
  // remove(test_bin);
  return 0;
}
