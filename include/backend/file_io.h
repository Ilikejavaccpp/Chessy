// For like stockfish, daily puzzles, etc.
// Will work on this and will NOT delete this "obsolete" file.
// Will use header guards only. Here, in this `backend/` space, we use this
// formal naming convention (also applies to files):
// `.h` -> C header files
// `.hpp` -> C++ header files
// includes are either...
// 1. outside the guard (if there is pragma or is a utils_)
// 2. inside the guard (else otherwise)
// params are either...
// 1. __snake_case (object)
// 2. snake_case or camelCase (normal)
// 3. _snake_case (private for compatibilty, e.g .`_this` for C++)
#ifndef CHESSY_BACKEND_C_CORE_FILEIO
#define CHESSY_BACKEND_C_CORE_FILEIO

// No mangling
#ifdef __cplusplus
extern "C" {
#endif

#include "smolInt.h" // also provides the c stdint library (stdint.h)
#include <stdio.h>

#define CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX 0x0B
#define CHESSY_BACKEND__SMALL_BATCH_COUNT_MAX SMOL_INT_LIBRARY__NIBBLE_MAXSIZE

/* For convenience's sake, make dedicated helper macros for
 * the signature mask and unmask */
#define CHESSY_BACKEND_CORE_FILEIO_MASK(x)                                     \
  (x ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX)
#define CHESSY_BACKEND_CORE_FILEIO_UNMASK(x)                                   \
  (x ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX)

/* Some aliases of the mask/unmasks */
#define CHESSY_BACKEND__MASKOB(x) CHESSY_BACKEND_CORE_FILEIO_MASK(x)
#define CHESSY_BACKEND__UNMASKOB(x) CHESSY_BACKEND_CORE_FILEIO_UNMASK(x)

/* Some helpers so that we don't get confused
 * and accidentally set off the wrong flag; e.g.
 * "w" or "b" instead of "wb"
 */

// write to the binary (initialize it)
void writeBinary(FILE **__restrict __overw_file,
                 const char *__restrict file_name);

// append to the binary (just put stuff at the back/newline)
void appendBinary(FILE **__restrict __overw_file,
                  const char *__restrict file_name);

// read from the binary
void readBinary(FILE **__restrict _input_file,
                const char *__restrict file_name);

// close the binary reading (deinitialize it)
void closeBinary(FILE **__restrict __close_file);

// close the binary reading (virtual buffer binary in RAM)
void closeVBinary(void **__restrict __close_vfile);

// test the binary
void testBinary(FILE **__restrict __test_file, uint8_t *out_flag);

/* Useful for Chess File saving and general text saving
 * on disk. These are for both... pgn/fen and board.h compatibility.
 * Also compatible with the standard strings*/

// pack a small batch of binary packed rle representation (0-63)
void packSmallBatch(const char *__restrict file_name,
                    uint8_t *__restrict batch_64);

// pack (append) a small batch of binary packed rle representation (0-63)
void packAppendSmallBatch(const char *__restrict file_name,
                          uint8_t *__restrict batch_64);

// pack a normal batch of binary packed rle representation (ASCII range, char
// range)
void packBatch(const char *__restrict file_name, int8_t *__restrict batch);

// pack a small batch of string to a file; useful for like usernames and short
// rle'd stuff (or no rle) like a fen sequence, etc.
void packMiniString(const char *__restrict file_name,
                    int8_t *__restrict batch_64);

/* Useful for Chess File loading saved and general text saved files
 * off disk. These are for both... pgn/fen and board.h compatibility.
 * Also compatible with files with the standard strings*/

// unpack a small batch of binary packed rle representation (0-63)
void unpackSmallBatch(const char *__restrict file_name,
                      uint8_t **__restrict out_batch_64);

// unpack a small batch of rle represention (0-63) from an indiced binary file
// via an index
void unpackSmallBatchIndexed(const char *__restrict file_name,
                             uint8_t **__restrict out_batch_64,
                             const uint32_t *__restrict chunk_jump);

// unpack a normal batch of binary packed rle representation (ASCII range, char
// range)
void unpackBatch(const char *__restrict file_name,
                 int8_t **__restrict out_batch);

// unpack a small batch of string binary packed normal represention (obfuscated
// obviously)
void unpackMiniString(const char *__restrict file_name,
                      int8_t **__restrict out_batch_64);

#if defined(CHESSY_BACKEND_C_CORE_FILEIO_FLAG__USE_CLEAR) ||                   \
    defined(CHESSY_BACKEND_FILEIO_FLAG__USE_CLEAR)

void initBinary(FILE **__restrict __overw_file, *__restrict file_name);
void _packBatch(const char *__restrict file_name, char *__restrict batch);
void _packSmallBatch(const char *__restrict file_name,
                     unsigned char *__restrict batch_64);
void _unpackBatch(const char *__restrict file_name,
                  char **__restrict out_batch);
void _unpackSmallBatch(const char *__restrict file_name,
                       unsigned char **__restrict out_batch_64);

#endif

#ifdef __cplusplus
}
#endif

#endif
