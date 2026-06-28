// TODO: implement this tomorrow on
// 29 Jun, 2026 Thu <- date tomm.
// Stockfish stuff == implement
/* Optional */
#define CHESSY_BACKEND_C_CORE_ENDEC_FLAG__USE_SNAME

#include "endec_board.h"
#include "file_io.h"

#include <malloc.h>
#include <string.h>

#if !defined(CHESSY_BACKEND_CORE_FLAG__USE_BOOL) &&                            \
    !defined(__FLAG__USE_STDBOOL_H)
/* Macros for stdbool.h, for legacy <C99 C compilers */

typedef _Bool bool;
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif

// NOTE: The following functions have logs
// - `encodeMatrix(..)`
// - `decodeMatrix(..)`

/* # EXPLANATION OF ENCODING-DECODING
 *
 * Encoding here, works via using nice special non-unicode, non-ascii
 * delimiters... For a normal chessy board_state batch... use nothing. The rest
 * below are the delimeter (`\128` or char with val integer char `128`) and
 * their associative tag + meaning:
 *
 * 0. 128+0          -> terminator [CRUCIAL] [CRITICAL]
 * 1. 128+d (depth)  -> for the depth of the engine lines. usually at the
 * beginning of a engine block
 * 2. 128+e (engine) -> for the engine lines
 * 3. 128+f (fen)    -> for the fen
 * 4. 128+p (pgn)    -> for the pgn
 * 5. 128+s (string) -> for a custom string (may be username, etc.); this means
 * that the file is a variable file and is overwritten many a times
 * 6. 128+S (Section)-> what section of chessy... every metadata of a section,
 * like practice, puzzles, etc. in *one* file... (uni big file approach)
 * [UNNEEDED] [EH]
 *
 * For `6.`, we may just use a multi variable file system with one big parent
 * file
 */

#if !defined(CHESSY_BACKEND_C_CORE_FILEIO_IMPLEMENTATION_C) &&                 \
    !defined(CHESSY_BACKEND_FILEIO_IMPLEMENTATION)

// A packed pair of integers sharing ONE single 8-bit byte.
// Perfect for layout states, UI grid coordinates, or paragraph flags.
struct PackedPair8 {
  uint8_t left : 4;  // 4 bits: range 0 to 15
  uint8_t right : 4; // 4 bits: range 0 to 15
};
struct DoubleByte {
  uint8_t left;
  uint8_t right;
};
struct MiniStringBlock {
  uint8_t count; // total count of characters
  int8_t __char; // the character pointed to currently
};
typedef struct MiniStringBlock MiniString; // unnecessary bloat

#endif

/* Helpers */

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
  long current_blocks = current_size / sizeof(PackedPair8);
  long missing_blocks = expected_blocks - current_blocks;

  struct PackedPair8 padding_block = {.left = 0, .right = 0};
  for (long i = 0; i < missing_blocks; i++) {
    fwrite(&padding_block, sizeof(PackedPair8), 1, f);
  }
  fclose(f);
}

/* Matrix (board) related functions (chessy board format/integer format) */

void encodeMatrix(const char *file_path, const uint8_t *board_64) {
  printf("[INFO] : Encoding matrix (board) with state... -> WAIT\n");
  packSmallBatch(
      file_path,
      (uint8_t *restrict)
          board_64); // remove `restrict` if don't want aggressive optimization.

  for (uint8_t i = 0; i < 64; ++i)
    printf((i % 8 == 7) ? " %d\n" : " %d", board_64[i]);

  printf("[INFO] : Finished encoding the matrix (board)\n");
}

void encodeAppendMatrix(const char *file_path, const uint8_t *board_64) {
  // Get the file size and index count for no. of blocks
  long file_size = get_file_size(file_path);
  long indices = file_size / (64 * sizeof(struct PackedPair8));

  // Append the small batch to the file
  packAppendSmallBatch(
      file_path,
      (uint8_t *restrict)
          board_64); // remove `restrict` if don't want aggressive optimization.

  // Calculate the number of encoded obfuscated tokens (via the pre'ed indices)
  // and set the padding for future indexing
  long target_blocks = (indices + 1) * 64;
  padToFixedBlockBoundary(file_path, target_blocks);
}

void decodeMatrix(const char *file_path, uint8_t **out_board_64) {
  printf("[INFO] : Unpacking matrix (board).. \n");
  unpackSmallBatch(file_path, out_board_64);
  printf("[INFO] : Unpacked matrix (board) with contents..\n");

  // Since it is packed, show the contents (successful or not)
  for (uint8_t i = 0; i < 64; ++i)
    printf((i % 8 == 7) ? " %d\n" : " %d", *out_board_64[i]);
}

void decodeMatrixIndexed(const char *file_path, uint8_t **out_board_64,
                         uint32_t board_index) {
  if (board_index == 0) {
    uint32_t __zero = 0;
    unpackSmallBatchIndexed(file_path, out_board_64, &__zero);
  } else
    unpackSmallBatchIndexed(file_path, out_board_64, &board_index);
}

/* Fen related stuff.. */

// Lookup table (private) for standard chessy board layout (integer)
// to FEN layout (used by stockfish, etc.)
static const char intToFen_lookup_table[] = {
    ' ',                          // EMPTY = 0
    'p', 'n', 'b', 'r', 'q', 'k', // Black pieces = 1-6
    'P', 'N', 'B', 'R', 'Q', 'K'  // White pieces = 7-12
};

// logic here, i forgot to implement, will do NOW
// TODO, implement fen here
// maybe stockfish too

void convert_boardToFen(char **__restrict out_fen_string,
                        uint8_t *__restrict board_64) {
  if (!out_fen_string || !board_64)
    return; // safety mechanism

  uint8_t i = 0; // the index (for characters..)

  for (uint8_t r = 0; r < 8; ++r) {
    uint8_t count_empty =
        0; // the empty count.. so that we can have a string part like
           // pppppppp/8/8/4P3/8 ... (i.e. PGN `1. e4` or UCI `e2e4`)

    for (uint8_t c = 0; c < 8; ++c) {
      uint8_t val =
          board_64[r * 8 + c]; // compiler will optimize the expression
                               // `r * 8` to `r << 3`. if not, (i.e. -O, -O0)
                               // then use (via ^X^V) the bitshift one provided
                               // in this comment block's above codefence no. 2

      if (val == 0)
        ++count_empty;
      else {
        *out_fen_string[i++] =
            '0' + count_empty; // the count of empty expressed in string via
                               // character / integer ASCII maths

        count_empty =
            0; // reset since we hit the piece (NOTE: we use postfix here for
               // the old return, aka background silent changing)
      }

      // Translate the integer into a fen literal via the lookup dictionary
      // (table)
      *out_fen_string[i++] = intToFen_lookup_table[val];
    }

    // Flush out the remaining spaces at the end of the row (`r`)
    if (count_empty > 0)
      *out_fen_string[i++] = '0' + count_empty;

    // Append a slash `/` delimiter to the end of every row (except the very
    // last, i.e array indice `7` or rank `1`)
    if (i < 7)
      *out_fen_string[i++] = '/';
  }

  // Properly append a null terminator, to safely terminate the string
  *out_fen_string[i] = '\0';
}

void convert_fenToBoard(char *__restrict fen_string,
                        uint8_t **__restrict out_board_64) {
  if (!fen_string || !out_board_64)
    return; // safety mechanism

  uint8_t r = 0; // the row
  uint8_t c = 0; // the col
  uint8_t i = 0; // the index (for characters..)

  // Clear (wipe) the board out so that we don't have to manually append stuff
  // and instead modify (increment) values.
  memset(out_board_64, 0, 64);

  // Scan the board until we hit a space or an EOF (the null terminator)
  while (fen_string[i] != '\0' && fen_string[i] != ' ') {
    char token = fen_string[i]; // the token
                                // used for piece type, empty or not... (just a
                                // normal token and maybe a pointless comment)

    if (token == '/') {
      r++;   // increment the row (i.e. move down)
      c = 0; // reset the column (in chess what we call a 'file')
    } else if (token >= '1' || token <= '8')
      c += (token - '0'); // conversion to an integer
                          // to skip the blanks, looking for a full
                          // for the piece type with color (`black / WHITE`)
    else {
      uint8_t id = 0; // initialize the variable that is going to hold the piece
                      // ID (0-12)

      // loop through all the possible id's and check if the token matches one.
      for (uint8_t p = 0; p < 12; ++p)
        if (token == intToFen_lookup_table[p]) {
          id = p;
          break;
        }

      // write to the board if there exists a piece
      // with some constraints for safety first.
      if (r >= 0 && r < 8 && c >= 0 && c < 8)
        *out_board_64[r * 8 + c] = id;

      // Move on to the next
      ++c;
    }
    // Increment the index
    ++i;
  }
}

/**/

/* The encoding / decoding for fen part */

/**/

void encodeFen(const char *file_path, int8_t *fen_string) {
  if (!file_path || !fen_string)
    return; // safety mechanism

  FILE *o_file = NULL;
  uint8_t error = 0;
  appendBinary(&o_file, file_path);
  testBinary(&o_file, &error);

  if (error != 0)
    return;

  // the beginning tag delimiter marker
  MiniString delim = {
      .count = 1,
      .__char = (int8_t)(128 ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX)};
  MiniString type_tag = {
      .count = 1,
      .__char = (int8_t)('f' ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX)};
  fwrite(&delim, sizeof(MiniString), 1, o_file);
  fwrite(&type_tag, sizeof(MiniString), 1, o_file);

  // write the fen payload
  uint8_t i = 0;
  while (fen_string[i] != '\0') {
    MiniString fen_block_part = {
        .count = 1,
        .__char =
            (int8_t)(fen_string[i] ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX)};
    fwrite(&fen_block_part, sizeof(struct MiniStringBlock), 1, o_file);
    i++;
  }

  // signature ending tag delimiter marker
  delim.count = 1,
  delim.__char = (int8_t)(128 ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX);
  MiniString term_tag = {
      .count = 1,
      .__char = (int8_t)('\0' ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX)};
  fwrite(&delim, sizeof(MiniString), 1, o_file);
  fwrite(&type_tag, sizeof(MiniString), 1, o_file);

  // close the file
  closeBinary(&o_file);
}

void decodeFen(const char *file_path, int8_t **out_fen) {
  if (!file_path || !out_fen)
    return; // safety mechanism

  FILE *i_file = NULL;
  uint8_t error;

  readBinary(&i_file, file_path);
  testBinary(&i_file, &error);

  /* Safely exit via making it a null pointer */
  if (!i_file)
    out_fen = NULL;

  if (error != 0)
    return; // safety mechanism

  char vbuf[CHESSY_BACKEND_C_CORE_FILEIO_FLAG__VBATCH_MAX] = {
      0}; // the temporary buffer (we are gonna append via this)

  uint8_t vbi = 0; // since the total number of characters (max) is 256
                   // (DEFAULT, i think 512 is a bit too large)

  _Bool verified = false; // make sure if we are at the right block

  fseek(i_file, 0, SEEK_END);
  size_t i_fsz = ftell(i_file); // calculate the file size at instant; ...
                                // the file size
  fseek(i_file, 0, SEEK_SET);

  if (i_fsz < (size_t)(sizeof(MiniString) * 2)) {
    closeBinary(&i_file);
    return; // since it obviously doesn't contain the necessary block.
            // may change this if you want to use multi object type fs
            // architecture
  }

  /* Allocate the file on the heap and get the number of bytes read. */
  uint8_t *i_fbuf = (uint8_t *)malloc(i_fsz);
  if (!i_fbuf) {
    closeBinary(&i_file);
    return;
  }

  size_t __r_bytes =
      fread(i_fbuf, 1, i_fsz, i_file); // the count of read bytes from the
                                       // allocated file buffer (`i_fbuf`)

  closeBinary(&i_file); // safely close the file

  // while (fread(vbuf, CHESSY_BACKEND_C_CORE_FILEIO_FLAG__VBATCH_MAX_SIZE, 1,
  //              i_file) == 1) {
  //   int8_t __char =
  //       section_block.__char ^
  //       CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX; // de-obfuscate it via our
  //                                              // signature unmask
  //
  //   /* Scan for the delimeter then, check if it is the `f`, aka flag no. 3.
  //    * See more about delimeter flags and weird stuff at the topmost
  //    explanatory
  //    * boilerplate comment. NOTE: if you are in Nvim... type `<Esc>gg` (to
  //    go up
  //    * and find it) or type `<Esc>/EXPLANATION FOR ENC<Enter>` then type
  //    `N`*/
  //   if ((uint8_t)__char ==
  //       128) { // 128 is the delimeter special code, again see more via the
  //              // instructions provided above this comment
  //   }
  // }

  /* Map out our memory pointers and do calculation */
  MiniString *blocks =
      (MiniString *)i_fbuf; // map over the allocated file array for the current
                            // pointer to point to the first index

  size_t count_blks =
      __r_bytes / sizeof(MiniString); // the count of the blocks.

  /* Begin calculation..
   * Loop through the contents then find the delimiter sequence */
  for (size_t i = 0; i < count_blks; ++i) {
    int8_t __char_entry =
        blocks[i].__char ^
        CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX; // de-obfuscate it via our
                                               // signature unmask

    /* Scan for the delimeter then, check if it is the `f`, aka flag no. 3.
     * See more about delimeter flags and weird stuff at the topmost
     * explanatory boilerplate comment.
     *
     * NOTE: if you are in Nvim... type `<Esc>gg` (to go up
     * and find it) or type `<Esc>/EXPLANATION FOR ENC<Enter>` then type `N`*/
    if ((uint8_t)__char_entry == 128 && (i + 1) < count_blks &&
        (blocks[i + 1].__char ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX) ==
            'f' // This is the next entry check (if it is `f`)

    ) { // 128 is the delimeter special code, again see
      // more via the instructions provided above this
      // comment. also we put a restraint so that we
      // don't have a read overflow (segfault)
      verified = true;
      i += 2; // skip the delimiter indentifier entry and go to the actual fen
              // data.

      /* Parse the payload and do actions to it */
      while (i < count_blks) {
        int8_t __char_data =
            blocks[i].__char ^
            CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX; // de-obfuscate the data via
                                                   // our signature unmask
                                                   // (again) :sigh:

        /* Look for the trailer terminating delimiter tag; i.e
         * `128` in char + `'\0'`.
         * See more about delimeter flags and weird stuff at the topmost
         * explanatory boilerplate comment.
         */
        if ((uint8_t)__char_data == 128 && (i + 1) < count_blks &&
            (blocks[i + 1].__char ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX) ==
                '\0' // This is the next entry check (if it is `\0`)

        )
          break; // we have reached the end of the fen. time to exit

        /* Unpack them */
        for (uint8_t j = 0; j < blocks[i].count; ++j)
          if (vbi < CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX -
                        1 // if the virtual buffer index is less than
                          // the virtual buffer's size (256), it is decremented
                          // exactly because the index is signed (starts from 0)
          )
            vbuf[vbi++] = __char_data;
        i++;
      }
      break; // succesfully finished extracting the target FEN
    }
  }

  /* Cleanup: free the leftover remnants, we don't want memory leaks and garbage
   * memory and also
   * Assignment: make sure that we modify the parameter (arg) passed, i.e the
   * fen string*/
  closeVBinary((void **)&i_fbuf); // same as `free(i_buf);`
  if (verified) {
    memcpy(*out_fen, vbuf, vbi);
    *out_fen[vbi] = '\0';
  }
}

/**/

/* Stockfish and engine related functions */

/**/

void encodeEngineLines(const char *file_path, int8_t *engine_lines,
                       uint8_t depth, uint16_t eval_score) {
  if (!file_path || !engine_lines)
    return;

  FILE *o_file;
  uint8_t error;

  appendBinary(&o_file, file_path);
  testBinary(&o_file, &error);

  if (error != 0)
    return; // safety mechanism

  /* Write the block's signature/ header sequence, also this is not Ai.
   * If you think it is so, then pls gtfo (Get-The-Frick-Out) */
  // the beginning tag delimiter marker
  MiniString delim = {
      .count = 1,
      .__char = (int8_t)(128 ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX)};
  MiniString type_tag = {
      .count = 1,
      .__char = (int8_t)('e' ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX)};
  fwrite(&delim, sizeof(MiniString), 1, o_file);
  fwrite(&type_tag, sizeof(MiniString), 1, o_file);

  /* Write the payload to disk */
  // First, configure the metadata
  ChessEngineEval mdata = {
      .validator = CHESSY_BACKEND__MASKOB(128),
      .eval_sc = 0,
      .depth = depth,
  };

  // Second, write the payload with the configured metadata

  /* Close off with the terminator signature */
  // signature ending tag delimiter marker
  delim.count = 1,
  delim.__char = (int8_t)(128 ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX);
  MiniString term_tag = {
      .count = 1,
      .__char = (int8_t)('\0' ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX)};
  fwrite(&delim, sizeof(MiniString), 1, o_file);
  fwrite(&type_tag, sizeof(MiniString), 1, o_file);

  /* Cleanup */
  // close the file
  closeBinary(&o_file);
}
