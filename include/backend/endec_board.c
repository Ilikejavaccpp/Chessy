// TODO: implement this tomorrow on
// 29 Jun, 2026 Thu <- date tomm.
// Stockfish stuff == implement
/* Optional */
#define CHESSY_BACKEND_C_CORE_ENDEC_FLAG__USE_SNAME

#include "endec_board.h"
#include "file_io.h"
#include "macros.h" // handy dandy
#include "vendor.h" // handy dandy (VENDOR utilities for C)

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

// A helper structure to measure exactly what the file driver is writing
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
        if (count_empty > 0) {
          (*out_fen_string)[i++] =
              '0' + count_empty; // the count of empty expressed in string via
                                 // character / integer ASCII maths
          count_empty =
              0; // reset since we hit the piece (NOTE: we use postfix here for
                 // the old return, aka background silent changing)
        }
        // Translate the integer into a fen literal via the lookup dictionary
        // (table)
        (*out_fen_string)[i++] = intToFen_lookup_table[val];
      }
    }

    // Flush out the remaining spaces at the end of the row (`r`)
    if (count_empty > 0)
      (*out_fen_string)[i++] = '0' + count_empty;

    // Append a slash `/` delimiter to the end of every row (except the very
    // last, i.e array indice `7` or rank `1`)
    if (r < 7)
      (*out_fen_string)[i++] = '/';
  }

  // Properly append a null terminator, to safely terminate the string
  (*out_fen_string)[i] = '\0';
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
  memset(*out_board_64, 0, 64);

  // Scan the board until we hit a space or an EOF (the null terminator)
  while (fen_string[i] != '\0' && fen_string[i] != ' ') {
    char token = fen_string[i]; // the token
                                // used for piece type, empty or not... (just a
                                // normal token and maybe a pointless comment)

    if (token == '/') {
      r++;   // increment the row (i.e. move down)
      c = 0; // reset the column (in chess what we call a 'file')
    } else if (token >= '1' && token <= '8')
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
        (*out_board_64)[r * 8 + c] = id;

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
  fwrite(&term_tag, sizeof(MiniString), 1, o_file);

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
                   // will (in the future) make some boilerplate via defining an
                   // almost identical clone that will just change the types of
                   // the indices

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
      .eval_sc = (uint16_t)eval_score,
      .depth = depth,
  };

  // Second, write the payload with the configured metadata
  fwrite(&mdata, sizeof(ChessEngineEval), 1,
         o_file); // first write the metadata
                  // i.e. the block header indicator

  uint16_t i = 0;
  while (engine_lines[i] != '\0') {
    if (engine_lines[i] ==
        '\n') /* If we hit a newline, write it as a standalone uncompressed
                 newline character (<CR> / <Enter>) for readability. Also makes
                 universal only encode ASCII clear.*/
    {
      MiniString end_line = {
          .count = 1,
          .__char = (int8_t)'\n',
      }; /* We named it as so since let's be real, END of the engine LINE.*/
      fwrite(&end_line, sizeof(MiniString), 1, o_file);
      i++;      /* Skip to the next */
      continue; /* Skip for this (don't encode it or do operations to it) */
    }

    /* Encode normal characters that are not newlines and
     * are basically uci. */
    MiniString block_char = {
        .count = 1,
        .__char = engine_lines[i],
    };

    while (
        engine_lines[i + 1] != '\0' && // While it is not the EOF of the string
        engine_lines[i] ==
            engine_lines[i + 1] && // same for the next (skipping and
                                   // effectiveness) #better comments
        block_char.count < 255 &&  // redundant but sure
        engine_lines[i + 1] !=
            '\n' // make sure this is the current line and not the next, don't
                 // multiply it (i.e.) the count
    ) {
      block_char.count++;
      i++;
    }

    /* Write the normal obfuscated string bit (MiniString) to the file */
    /* Via masking -> writing -> updating */
    block_char.__char = (int8_t)CHESSY_BACKEND__MASKOB(block_char.__char);
    fwrite(&block_char, sizeof(MiniString), 1, o_file);
    i++;
  }

  /* Close off with the terminator signature */
  // signature ending tag delimiter marker
  delim.count = 1,
  delim.__char = (int8_t)(128 ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX);
  MiniString term_tag = {
      .count = 1,
      .__char = (int8_t)('\0' ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX)};
  fwrite(&delim, sizeof(MiniString), 1, o_file);
  fwrite(&term_tag, sizeof(MiniString), 1, o_file);

  /* Cleanup */
  // close the file
  closeBinary(&o_file);
}

void decodeEngineLines(const char *file_path, int8_t **restrict out_lines,
                       uint8_t *restrict depth,
                       uint16_t **restrict eval_scores) {
  if (!file_path || !out_lines || !depth || !eval_scores)
    return; // safety mechanism

  FILE *i_file = NULL;
  uint8_t error;

  readBinary(&i_file, file_path);
  testBinary(&i_file, &error);

  /* The first `if` is for people who are new to this codebase/forgot some stuff
   * about the functions here */
  if (!i_file)
    return; // there is no such thing
  if (error != 0)
    return; // safely exit

  char vbuf[CHESSY_BACKEND_C_CORE_FILEIO_FLAG__VBATCH_MAX * 2] = {0};
  uint16_t vbi =
      0; // since the total number of characters (max) is 256*2 == 512
  bool verified = false;

  fseek(i_file, 0, SEEK_END);
  size_t i_fsz = ftell(i_file);
  fseek(i_file, 0, SEEK_SET);

  /* Allocate the file on the heap and get the number of bytes read. */
  uint8_t *i_fbuf = (uint8_t *)malloc(i_fsz);
  if (!i_fbuf) {
    *out_lines = NULL; // now, non-optional
    closeBinary(&i_file);
    return;
  }

  size_t __r_bytes =
      fread(i_fbuf, 1, i_fsz, i_file); // the count of read bytes from the
                                       // allocated file buffer (`i_fbuf`)

  closeBinary(&i_file); // safely close the file

  /* Map out our memory pointers and do calculation */
  MiniString *blocks =
      (MiniString *)i_fbuf; // map over the allocated file array for the current
                            // pointer to point to the first index

  size_t count_blks =
      __r_bytes / sizeof(MiniString); // the count of the blocks.

  /* Begin calculation..
   * Loop through the contents then find the delimiter sequence */
  for (size_t _i = 0; _i < count_blks; ++_i) {
    int8_t __char_entry =
        blocks[_i].__char ^
        CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX; // de-obfuscate it via our
                                               // signature unmask

    /* Scan for the delimeter then, check if it is the `e`, aka flag no. 2.
     * See more about delimeter flags and weird stuff at the topmost
     * explanatory boilerplate comment.
     *
     * NOTE: if you are in Nvim... type `<Esc>gg` (to go up
     * and find it) or type `<Esc>/EXPLANATION FOR ENC<Enter>` then type `N`*/
    if ((uint8_t)__char_entry == 128 && (_i + 1) < count_blks &&
        (blocks[_i + 1].__char ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX) ==
            'e' // This is the next entry check (if it is `e`)

    ) { // 128 is the delimeter special code, again see
        // more via the instructions provided above this
        // comment. also we put a restraint so that we
        // don't have a read overflow (segfault)

      verified = true;
      uint8_t line_no = 0; // For various eval scores and move sequences

      ChessEngineEvalFileData *meta =
          (ChessEngineEval
               *)&blocks[_i + 2]; // extract shit right after the two signifiers
      *depth = meta->depth;       // assign

      // BUG, REDUNDANT
      // size_t bytes_to_skip =
      //     (2 * sizeof(MiniString)) + sizeof(struct ChessEngineEvalMetaData);
      uint8_t *payload_addr =
          (uint8_t *)&blocks[_i + 2] +
          sizeof(ChessEngineEval); // if the compiler is a karen
      size_t i = (MiniString *)payload_addr - blocks; // elements to skip
      // DANGEROUS
      // i += 2 + sizeof(ChessEngineEvalFileData) /
      //              sizeof(MiniString); // skip the delimiter indentifier
      //              entry
      //                                  // and go to the actual line(s) data.

      /* Parse the payload and do actions to it */
      while (i < count_blks) {
        bool isTag = false;

        /* Look for the trailer terminating delimiter tag; i.e
         * `128` in char + `'\0'`.
         * See more about delimeter flags and weird stuff at the topmost
         * explanatory boilerplate comment.
         */
        // DEPECRATED, Explanations here but the inner logic (math) is broken
        // ;-;
        // if ((uint8_t)__char_data == 128 && (i + 1) < count_blks) {
        //   if ((blocks[i + 1].__char ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX)
        //   ==
        //       '\0' // This is the next entry check (if it is `\0`)
        //
        //   )
        //     break; // we have reached the end of the engine lines. time to
        //     exit
        //   elif ((blocks[i + 1].__char ^
        //          CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX) == 'E') {
        //     // (*eval_scores)[line_no] = (uint16_t)*(uint16_t *)(&blocks[i +
        //     // 2]);
        //     if (*eval_scores) {
        //       (*eval_scores)[line_no] = *((uint16_t *restrict)&blocks[i +
        //       2]);
        //     }
        //     i += 3; // because of tag shit,
        //             // 1 == delimeter
        //             // 1 == the id (i.e 'E')
        //             // 1 == the score
        //     continue;
        //   }
        // }
        // Force the check to ONLY trigger if the block is a single-byte
        // structural flag
        // FIX, CHANGELOG --> don't use the __char_data
        // as it may retain garbage memory (easy to forget)
        if ((uint8_t)blocks[i].__char == CHESSY_BACKEND__MASKOB(128) &&
            blocks[i].count == 1 && (i + 1) < count_blks) {
          int8_t next_tag =
              blocks[i + 1].__char ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX;

          if (next_tag == '\0') {
            i += 2; // make sure both are cut off.
                    // X ->we increment `i` at the bottom so no need for an
                    // extra 1. X ->also, readability issues & debugging
                    // nightmare
            break;  // reached end of engine lines, exit loop cleanly
          }
          elif (next_tag == 'E') {
            isTag = true;
            if (*eval_scores) {
              // REDUNDANT, BUG, DANGEROUS
              // (*eval_scores)[line_no] = *(
              //     (uint16_t *restrict)&blocks[i + 2]); // simple casting
              //                                          // that doesn't work
              //                                          // (truncates `e` in
              //                                          the
              //                                          // test for some
              //                                          reason)
              uint16_t eval_score_tempv;
              memcpy(&eval_score_tempv, &blocks[i + 2], sizeof(uint16_t));
              if (line_no > 0 && (line_no - 1) < 3) {
                (*eval_scores)[line_no - 1] = eval_score_tempv;
              } else if (line_no == 0) {
                (*eval_scores)[0] = eval_score_tempv;
              }
            }

            i += 3;   // skip the 3— wow an em dash
                      // [DELIMETER 128] [FLAG 'E'] [NEWLINE]
            continue; // skip this
          }
          // else goto dec_payload_for_fix;
        }
        if (!isTag) {
          // dec_payload_for_fix:;

          int8_t __char_data;
          if (blocks[i].__char == '\n') {
            __char_data = blocks[i].__char;
            ++line_no;
          } else
            __char_data = CHESSY_BACKEND__UNMASKOB(
                blocks[i].__char); // de-obfuscate the data
                                   // via our signature
                                   // unmask (again) :sigh:
          fprintf(stderr,
                  "[DEBUG] i=%zu, raw=%02X, decoded='%c' (%d), line=%d\n", i,
                  (uint8_t)blocks[i].__char, __char_data, __char_data, line_no);
          /* Unpack them */
          for (uint8_t j = 0; j < blocks[i].count; ++j)
            if (vbi < (CHESSY_BACKEND_C_CORE_FILEIO_FLAG__VBATCH_MAX * 2) -
                          1 // if the virtual buffer index is less than
                            // the virtual buffer's size (512), it is
                            // decremented exactly because the index is signed
                            // (starts from 0) Note here, `decremented` in this
                            // comment means `const x - 1`
                            //
                            // NOTE: this was the FIXME bug.
            )
              vbuf[vbi++] = __char_data;
          i++;
        }
      }
      break; // succesfully finished extracting the target engine lines
    }
  }

  /* Cleanup: free the leftover remnants, we don't want memory leaks and garbage
   * memory and also
   * Assignment: make sure that we modify the parameter (arg) passed, i.e the
   * fen string*/
  closeVBinary((void **)&i_fbuf); // same as `free(i_buf);`
  if (verified && vbi > 0) {
    *out_lines = malloc(vbi + 1);
    if (*out_lines) {
      memcpy(*out_lines, vbuf, vbi);
      (*out_lines)[vbi] = '\0';
    }
  } else
    *out_lines = NULL;
}

static inline void __reset_coord_brd(uint8_t *board_64) {
  memset(board_64, 0, 64); // fresh wipe
}

static inline void __reset_coord_brd_startpos(uint8_t *board_64) {
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

static inline void __reset_coord_brd_brd(uint8_t *board_64,
                                         const uint8_t *custom_brd) {
  if (board_64 && custom_brd)
    memcpy(board_64, custom_brd, 64);
}

static inline bool __check_is_brd_empty(uint8_t *_Nonnull board_64) {
  // A clean reference block of 64 zeroes
  static const uint8_t empty_reference[64] = {0};

  // memcmp returns 0 if all 64 bytes match the reference perfectly
  return memcmp(board_64, empty_reference, 64) == 0;
}

/**/

/* These don't work and thus pls add/remove modifications to them */

/**/

void convert_pgnToUCI(int8_t *restrict *out_uci, int8_t *restrict pgn_lines,
                      const char *mode, const uint8_t *start_pos_board) {
  if (!out_uci || !pgn_lines)
    return; // safety mechanism

  /* Check if the pointer (to be output) has garbage memory and thus clear it */
  if (*out_uci == NULL) {
    *out_uci = malloc(CHESSY_BACKEND_C_CORE_FILEIO_FLAG__VBATCH_MAX * 2);
    if (!*out_uci)
      return; // safety mechanism
  }

  /* Check if start_pos_board was passed and/or if it was just an empty board */
  // TODO: fix overwrite of const
  if (start_pos_board && __check_is_brd_empty((uint8_t *)start_pos_board))
    return; // no need to decode just a string
  elif (!start_pos_board)
      //
      // memcpy((uint8_t *)start_pos_board,
      //      CHESSY_BACKEND__DEFAULT_MODE_CHS_STARTPOS, 64);
      __reset_coord_brd_startpos((uint8_t *)start_pos_board);

  /* Begin */
  uint16_t i =
      0; // index for characters
         // UCI lines (engine lines) are like at most (for any sane person)
         // 256 (vbatch) * 2 (double safety + padding) == 512 (_ > 5*5*16)

  /* NOTE (TODO)
   * LOGIC:
   *
   * simply start out with a pgn line as
   * 1. e4 e5 2. Nf3 .. \n
   * 1. d4 d5 2. c4 ..
   * ..
   *
   * here, if we hit a newline character.. then append it, else for
   * every other sequence (and space), track the initials then finals..
   *
   * so we need a board variable, know the board and map shit like this
   * 1. X .. -> X piece is white, find its coordinates.. (board_t, board_f ->
   * *T*hen (before) *F*inal)
   * 1. .. X -> X piece is black, same function.
   *
   *  NOTE: remove the move indicator or store it in something (maybe a global
   * inline via getter/setters)
   *
   * first, calculate pgn average line min-max.
   * So, basically in a pgn packet (without headers and shit) we have
   * [MOVE_NUMBER] [MOVE_LOC WHITE (e.g. Nf3xd4, aka 6 (remove `x` we get 5,
   * since nowadays people use `x` as in `e5xd4` use 6) == 1] [SPACE == 1]
   * [MOVE_LOC BLACK == 6]
   * */

  // mode = "ox";
  // .
  uint8_t __mode_size = strlen(mode);
  uint16_t __pgn_ln_size_max = 0; // temporary
  char __nl_mode = (char)*CHESSY_BACKEND__DEFAULT_MODE_NL;
  int8_t **uci_stuff = {NULL}; // dummy

  /* Newline formatting payload configuration */
  if (__nl_mode == (char)*CHESSY_BACKEND__MODE_NL_NS) // "s"
  {
    // .. do shit here

    // I am stuck pls help me
    // strstr(pgn_lines, "\n"); /* find any newline occurence */

    /* PIPELINE:
     * Consider a move (white + black) as a packet for converting
     * Then get the index for a newline.. (upto which via the packet magic)
     * with that knowledge convert the others till that. append the newline to
     * the out_uci then do shit.*/
  }
  elif (__nl_mode == (char)*CHESSY_BACKEND__MODE_NL_WS) {
    // .. do shit here
  }

  /* Specifies the mode -- oldschool, normal or default */
  /* Since the default really is simply either of these, no need to
   * write another obsolete junk code block */
  if (strstr(mode, "o")) {
    info_log("LEX -- parse:: gonna do oldschool parsing");
  }
  elif (strstr(
      mode, CHESSY_BACKEND__MODE_PGN_NOTATION_NML)) { /* The main parsing action
                                                         happens here */

  }

  trace_log("STATE -- variable `mode` in function `convert_pgnToUCI()`.. "
            "length=%d string=\"%s\"",
            __mode_size, mode);

  /* While loop for computation */
  while (true) {
    // do nothing
    // this will eat resources but fck it
    break; // no crash
  }

  /* Cleanup */
  (*out_uci)[i] = '\0'; // properly terminate it
  free(*out_uci);
}

void convert_uciToPGN(int8_t *restrict *out_pgn, int8_t *restrict uci_lines,
                      const uint8_t *start_pos_board) {
  if (!out_pgn || !uci_lines)
    return; // safety mechanism

  // temporary, use almost the same logic as before but reverse functional steps
  return;
}

/* include the experimentals too
 * Here, it is the `ai` prefix meaning ya guessed it:
 * AI generated (++some human/dev modifications) */
#include "experimental/ai.endec_board.experimental.c"
