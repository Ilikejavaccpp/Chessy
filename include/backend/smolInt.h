#pragma once // since many people are gonna hate me.
             // I used a stdc lib so hence must stick with stuff.
             // Also this file is considered a library file (outside)
             // and thus defining flags is tedious. own problems should be dealt
             // with not foreigner's ones.

// May modify these 3 lines for C/C++ support (namespace + C-styled shenanigans)
// Change the line no. to the current line. For ease of use, there's one non
// character comment separating this and the guard, hence...
// comment/uncommenting is easy (no mistakes)
//
#ifdef SMOL_INT_LIBRARY_HEADER_C
#error "Compilation error: Included the smol int library twice!\n \
        In included file `smolInt.h`...\n \
        In lines 12-21: `#ifndef SMOL_INT_LIB...` \n \
        Fix: Please remove the included file *once* or remove/modify \
        this text directly in `smolInt.h` \n \
        \n \
        [ERROR] : Included the library twice.\n \
        [FIX]   : Remove the included `smolInt.h` or modify this.\n"
#endif

#ifndef SMOL_INT_LIBRARY_HEADER_C
#define SMOL_INT_LIBRARY_HEADER_C

/* Nice preprocessor definitions / constants so that
 * we don't make mistakes (dangerous, semantic) (especially you, the reviewer)
 */

#define SMOL_INT_LIBRARY__NIBBLE_MAXSIZE 0x0F
#define SMOL_INT_LIBRARY__NIBBLE_MAX SMOL_INT_LIBRARY__NIBBLE_MAXSIZE // alias

#define SMOL_INT_LIBRARY__CHUNKT_MAXSIZE 0x7FFF
#define SMOL_INT_LIBRARY__CHUNKT_MAX SMOL_INT_LIBRARY__CHUNKT_MAXSIZE
#define SMOL_INT_LIBRARY__CHUNK_INT_T__MAXSIZE SMOL_INT_LIBRARY__CHUNKT_MAXSIZE
#define SMOL_INT_LIBRARY__CHUNK_INT_T__MAX SMOL_INT_LIBRARY__CHUNKT_MAXSIZE

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Basically an alias but helps for new people (who think that
// characters aren't == to an integer)
typedef uint_fast8_t bite_t;

// A packed pair of integers sharing ONE single 8-bit byte.
// Perfect for layout states, UI grid coordinates, or paragraph flags.
typedef struct PackedPair8 PackedPair8;

// The Nibble Int (4-bit integer: 0 to 15)
// Takes up 1 byte standalone due to alignment, but array-packable!
typedef struct nibble_t nibble_t;

// The Chunk (of an) Int (15-bit signed integer: 0 to 32K)
// Great for Arduino projects and when you feel an int is
// way too big.
typedef struct chunkint_t chunkint_t;

// A byte sized flag container
// The True Smol Int (6-bit integer: 0 to 63)
// Leaves 2 bits for hardware status flags inside the same byte!
typedef struct FlaggedByte FlaggedByte;

// A byte size flag container
typedef struct FlaggedByte fByte_t;

// A double byte
typedef struct DoubleByte DoubleByte;

// A double byte
typedef struct DoubleByte dByte_t;

// large file data.
// use this at your own cost.
typedef struct DoubleByte lFData_t;

/********************************************************
 * Functions required for the `nibble_t` struct to work
 ********************************************************/
// This is so that we don't get conflicts when you actually name
// your function for a character that allows nibbling.
void initNibbleint(nibble_t *_this);

// Increment operator (++var)
nibble_t *SmolInt_incPre_nibble(nibble_t *_this);

// Increment operator (var++)
nibble_t *SmolInt_incPost_nibble(nibble_t *_this);

// Decrement operator (--var)
nibble_t *SmolInt_decPre_nibble(nibble_t *_this);

// Decrement operator (var--)
nibble_t *SmolInt_decPost_nibble(nibble_t *_this);

// Force bits to be on in a specific order
void SmolInt_forceBitsOn_nibble(nibble_t *_this, uint16_t _mask);

// Force bits to be off in a specific order
void SmolInt_forceBitsOff_nibble(nibble_t *_this, uint16_t _mask);

// Equals sign operation related shenanigans
// Assignment
// Assigns one nibble to another chunkint.
nibble_t *SmolInt_isAssign_nibble(nibble_t *_this, nibble_t *_other);

#ifdef __cplusplus
// Equals sign operation related shenanigans
// Comparision
// Checks if one nibble is equals to the other.
bool SmolInt_isEquals_nibble(nibble_t *_this, nibble_t *_other);

// Test the bits (if they are all 1 or not)
bool SmolInt_testBits_nibble(nibble_t *_this, nibble_t *_other);
#else
// Equals sign operation related shenanigans
// Comparision
// Checks if one nibble is equals to the other.
_Bool SmolInt_isEquals_nibble(nibble_t *_this, nibble_t *_other);

// Test the bits (if they are all 1 or not)
_Bool SmolInt_testBits_nibble(nibble_t *_this, nibble_t *_other);
#endif

/********************************************************
 * Functions required for the `chunkint_t` struct to work
 ********************************************************/
// Function for initializing your chunk of an int (chunkint_t)
void initChunkint(chunkint_t *_this);

// Increment operator (++var)
chunkint_t *SmolInt_incPre_chunkint(chunkint_t *_this);

// Increment operator (var++)
chunkint_t *SmolInt_incPost_chunkint(chunkint_t *_this);

// Decrement operator (--var)
chunkint_t *SmolInt_decPre_chunkint(chunkint_t *_this);

// Decrement operator (var--)
chunkint_t *SmolInt_decPost_chunkint(chunkint_t *_this);

// Force bits to be on in a specific order
void SmolInt_forceBitsOn_chunkint(chunkint_t *_this, uint16_t _mask);

// Force bits to be off in a specific order
void SmolInt_forceBitsOff_chunkint(chunkint_t *_this, uint16_t _mask);

// Equals sign operation related shenanigans
// Assignment
// Assigns one chunkint to another chunkint.
chunkint_t *SmolInt_isAssign_chunkint(chunkint_t *_this, chunkint_t *_other);

#ifdef __cplusplus
// Equals sign operation related shenanigans
// Comparision
// Checks if one chunkint is equals to the other.
bool SmolInt_isEquals_chunkint(chunkint_t *_this, chunkint_t *_other);

// Test the bits (if they are all 1 or not)
bool SmolInt_testBits_chunkint(chunkint_t *_this, chunkint_t *_other);
#else
// Equals sign operation related shenanigans
// Comparision
// Checks if one chunkint is equals to the other.
_Bool SmolInt_isEquals_chunkint(chunkint_t *_this, chunkint_t *_other);

// Test the bits (if they are all 1 or not)
_Bool SmolInt_testBits_chunkint(chunkint_t *_this, chunkint_t *_other);
#endif

#ifdef SMOL_INT_LIBRARY_FLAG__USE_SNAME
// Basically an alias but helps for new people (who think that
// characters aren't == to an integer)
typedef uint_fast8_t bite;
#endif

#ifdef __cplusplus
}
#endif

#endif
