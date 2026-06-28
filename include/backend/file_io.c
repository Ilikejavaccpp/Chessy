// Please don't mess with the C files.
// Rewriting is hard and time consuming
#include "file_io.h"
#include "smolInt.h"
#include <malloc.h>

// Raylib style, not clay one.
// Here is the commented clay one (`// <- headg CLAY`)
// #if defined(CHESSY_BACKEND_C_CORE_FILEIO_IMPLEMENTATION_C) ||
// (CHESSY_BACKEND_FILEIO_IMPLEMENTATION)
#if !defined(CHESSY_BACKEND_C_CORE_FILEIO_IMPLEMENTATION_C) &&                 \
    !defined(CHESSY_BACKEND_FILEIO_IMPLEMENTATION)
#define CHESSY_BACKEND_C_CORE_FILEIO_IMPLEMENTATION_C
#define CHESSY_BACKEND_FILEIO_IMPLEMENTATION

#define CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX 0x0B
#define CHESSY_BACKEND__SMALL_BATCH_COUNT_MAX SMOL_INT_LIBRARY__NIBBLE_MAXSIZE

#define CHESSY_BACKEND__SMALL_BATCH_

/* Functions stolen/borrowed (Vendor)... */

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

/* No comments for the intro (already did them in the headers).
 * Only comments for the body for explanations. Brief up (TODO).*/
void writeBinary(FILE **restrict __overw_file, const char *restrict file_name) {
  // w for 'write'
  // b for 'binary'
  *__overw_file = fopen(file_name, "wb");
}
void appendBinary(FILE **restrict __overw_file,
                  const char *restrict file_name) {
  *__overw_file = fopen(file_name, "ab");
}
void readBinary(FILE **restrict __read_file, const char *restrict file_name) {
  // r for 'read'
  // b for 'binary'
  *__read_file = fopen(file_name, "rb");
}
void closeBinary(FILE **restrict __close_file) { fclose(*__close_file); }
void closeVBinary(void **restrict __close_vfile) { free(*__close_vfile); }
void testBinary(FILE **restrict __test_file, uint8_t *out_flag) {
  // if there is no file, aka `NULL`/`nullptr`
  if (!__test_file) {
    printf("[WARNING] : File is possibly corrupted. Cannot open\n");

#ifdef CHESSY_BACKEND_FILEIO_FLAG__USE_EWARNING
    printf("[ERROR] : File is possibly corrupted. Cannot open\n");
    exit(-1); // may remove this
#endif

    *out_flag = -1;
    return; // exit
  }

  out_flag = 0;
  return;
}

void packSmallBatch(const char *file_name, uint8_t *restrict batch_64) {
  FILE *o_file = NULL; // here the output file, aka the file which we are
                       // going to write the rle to.
  uint8_t error;
  writeBinary(&o_file, file_name);
  testBinary(&o_file, &error); // will automatically assign stuff

  if (error != 0)
    return; // exit if errors,
            // permissions, disk full, etc.

  struct PackedPair8
      block; // declare the file's block as this.
             // .`left` will be used for the count (like how many)
             // .`right` will be used for the value (here standard stuff
             // that the decoder will then do..)

  for (bite_t i = 0; i < 64; ++i) {
    uint8_t pointer = batch_64[i]; // the current piece / token
                                   // (we are pointing to)

    block.left = 1; // begin...

    while (
        batch_64[i] == batch_64[i + 1] && (bite_t)(i + 1) < 64 &&
        block.left < 15 && // this is because of the bit (uint8_t : 4, close to
                           // `nibble_t`)'s limit
        (i % 8 !=
         7) // this is for chess and 8x8 board matrixes, feel free to tweak it
    ) {
      block.left++; // skip the lookup and add one to the count
      i++;          // skip the lookup and increment twice in this frame
    }

    block.right = (pointer ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX) &
                  CHESSY_BACKEND__SMALL_BATCH_COUNT_MAX; // mask them

    fwrite(&block, sizeof(struct PackedPair8), 1,
           o_file); // write this rle block to the file (1 quantity)
                    // and thus flush our current block
  }

  closeBinary(&o_file); // close
}
void packAppendSmallBatch(const char *file_name, uint8_t *restrict batch_64) {
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
void packBatch(const char *file_name, int8_t *restrict batch) {
  FILE *o_file = NULL; // here the output file, aka the file which we are
                       // going to write the rle to.
  uint8_t error;
  writeBinary(&o_file, file_name);
  testBinary(&o_file, &error); // will automatically assign stuff

  if (error != 0)
    return; // exit if errors,
            // permissions, disk full, etc.

  dByte_t block; // declare the file's block as this.
                 // .`left` will be used for the count (like how many)
                 // .`right` will be used for the value (here standard stuff
                 // that the decoder will then do..)

  for (bite_t i = 0; batch[i] != '\0'; ++i) {
    block.right = batch[i]; // pointer
    block.left = 1;         // begin...

    while (batch[i] == batch[i + 1] && (bite_t)(i + 1) < 64 &&
           block.left < 255 // this is because of the bit (uint8_t : 4, close to
                            // `byte_t`)'s limit, aka chunkint limit | 7FFFF
                            // may use a preprocessor define / constant like
                            // `SMOL_INT_LIBRARY__CHUNKT_MAX | 7F000`
    ) {
      block.left++; // skip the lookup and add one to the count
      i++;          // skip the lookup and increment twice in this frame
    }
    fwrite(&block, sizeof(struct DoubleByte), 1, o_file);
  }

  closeBinary(&o_file);
}
void packMiniString(const char *file_name, int8_t *restrict batch_256) {
  FILE *o_file = NULL; // here the output file, aka the file which we are
                       // going to write the rle to.
  uint8_t error;
  writeBinary(&o_file, file_name);
  testBinary(&o_file, &error); // will automatically assign stuff

  if (error != 0)
    return; // exit if errors,
            // permissions, disk full, etc.

  struct MiniStringBlock
      block; // declare the file's block as this.
             // .`left` will be used for the count (like how many)
             // .`right` will be used for the value (here standard stuff
             // that the decoder will then do..)

  for (bite_t i = 0; batch_256[i] != '\0'; ++i) {
    int8_t pointer = batch_256[i]; // the current piece / token
                                   // (we are pointing to)

    block.count = 1; // begin...

    // Many of y'all may not get the tiny difference between this and rle
    // so to clarify, i am using the variable `pointer` and since we introduced
    // new stuff yeah.
    while (batch_256[i + block.count] == pointer &&
           block.count < 255 // this is because of the bit (uint8_t
                             // )'s limit
    )
      block.count++; // skip the lookup and add one to the count

    block.__char = (pointer ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX) &
                   CHESSY_BACKEND__SMALL_BATCH_COUNT_MAX; // mask them

    fwrite(&block, sizeof(MiniString), 1,
           o_file); // write this string block to the file (1 quantity)
                    // and thus flush our current block

    i += (block.count - 1); // synchronization
  }

  closeBinary(&o_file); // close
}

void unpackSmallBatch(const char *restrict file_name,
                      uint8_t **restrict out_batch_64) {
  FILE *i_file = NULL;
  uint8_t error;
  readBinary(&i_file, file_name);
  testBinary(&i_file, &error);

  if (error != 0)
    return; // error

  PackedPair8 block;
  bite_t i = 0; // index of the block, must make it up to 63

  while (fread(&block, sizeof(PackedPair8), 1, i_file) == 1) {
    // original count of id from the block
    uint8_t count = block.left;

    // unmask to get the id back from them
    uint8_t id = (block.right ^ CHESSY_BACKEND__SMALL_BATCH_VALUE_MAX) &
                 CHESSY_BACKEND__SMALL_BATCH_COUNT_MAX;

    // now, reverse rle
    // first loop through (reversely, here the for
    // is an inner loop) using the count
    for (uint8_t j = 0; j < count; ++j) {

      // if i is less than 64, the value of `fByte_t.value`
      // then append to the value.
      // we can then increase the i afterwards, like what we did in the rle loop
      if (i < 64)
        *out_batch_64[i++] = id;
    }
  }

  closeBinary(&i_file);
}
void unpackSmallBatchIndexed(const char *restrict file_name,
                             uint8_t **restrict out_batch_64,
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
        *out_batch_64[i++] = id;
      }
    }
    if (i >= 64)
      break; // Finished building target block chunk boundary
  }

  closeBinary(&i_file);
}
void unpackBatch(const char *restrict file_name, int8_t **restrict out_batch) {
  FILE *i_file;
  uint8_t error;
  readBinary(&i_file, file_name);
  testBinary(&i_file, &error);

  dByte_t block;
  uint_fast16_t i = 0; // index for the block

  while (fread(&block, sizeof(dByte_t), 1, i_file) == 1) {
    for (uint_fast16_t j = 0; j < block.left; ++j) {
      *out_batch[i++] =
          (int8_t)block.right; // since we are reading off individual
                               // characters aka `int8_t`
    }
  }

  *out_batch[i] = '\0'; // terminate the string by appending a NULL character

  closeBinary(&i_file);
}
void unpackMiniString(const char *restrict file_name,
                      int8_t **restrict out_batch_64) {
  FILE *i_file = NULL;
  uint8_t error = 0;
  readBinary(&i_file, file_name);
  testBinary(&i_file, &error);

  if (error != 0)
    return;

  MiniString block;    // much cleaner, just an alias
  uint_fast16_t i = 0; // index

  while (fread(&block, sizeof(MiniString), 1, i_file) == 1) {
    for (uint8_t j = 0; j < block.count; ++j)
      *out_batch_64[i++] = (int8_t)((uint8_t)block.__char ^ 0x0B);
  }

  *out_batch_64[i] =
      '\0'; // terminate the string with a `NULL`/`nullptr` character
  closeBinary(&i_file);
}

// #endif // <- headg CLAY
#endif
