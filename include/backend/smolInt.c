// Please don't mess with the C files.
// Rewriting is hard and time consuming
#include "smolInt.h"

// Raylib style, not clay one.
// Here is the commented clay one (`// <- headg CLAY`)
// #if defined(SMOL_INT_LIBRARY_IMPLEMENTATION_C) || (SMOL_INT_IMPLEMENTATION)
#if !defined(SMOL_INT_LIBRARY_IMPLEMENTATION_C) &&                             \
    !defined(SMOL_INT_IMPLEMENTATION)
#define SMOL_INT_LIBRARY_IMPLEMENTATION_C
#define SMOL_INT_IMPLEMENTATION

// A packed pair of integers sharing ONE single 8-bit byte.
// Perfect for layout states, UI grid coordinates, or paragraph flags.
struct PackedPair8 {
  uint8_t left : 4;  // 4 bits: range 0 to 15
  uint8_t right : 4; // 4 bits: range 0 to 15
};

// The Nibble Int (4-bit integer: 0 to 15)
// Takes up 1 byte standalone due to alignment, but array-packable!
struct nibble_t {
  uint8_t nibble_value : 4;
};

void initNibbleint(nibble_t *this) { this->nibble_value = 0; }

nibble_t *SmolInt_incPre_nibble(nibble_t *_this) {
  _this->nibble_value = (_this->nibble_value + 1) & 0x0F;
  return _this;
}
nibble_t *SmolInt_incPost_nibble(nibble_t *this) {
  nibble_t *__old = this; // heap cuz why not?
                          // let's do a bit of contradiction.
                          // I recently read a book about chemistry and the
                          // countless exceptions, so... have luck!
  this->nibble_value = (this->nibble_value + 1) & 0x0F;
  return __old;
}

nibble_t *SmolInt_decPre_nibble(nibble_t *this) {
  this->nibble_value = (this->nibble_value - 1) & 0x0F;
  return this;
}
nibble_t *SmolInt_decPost_nibble(nibble_t *this) {
  nibble_t *__old = this; // heap cuz why not?
                          // let's do a bit of contradiction.
                          // I recently read a book about chemistry and the
                          // countless exceptions, so... have luck!
  this->nibble_value = (this->nibble_value - 1) & 0x0F;
  return __old;
}

void SmolInt_forceBitsOn_nibble(nibble_t *this, uint16_t mask) {
  this->nibble_value = (this->nibble_value | mask) & 0x0F;
}
void SmolInt_forceBitsOff_nibble(nibble_t *this, uint16_t mask) {
  this->nibble_value = (this->nibble_value & ~mask) & 0x0F;
}

nibble_t *SmolInt_isAssign_nibble(nibble_t *this, nibble_t *other) {
  this->nibble_value = other->nibble_value & 0x0F;
  return this;
}
_Bool SmolInt_isEquals_nibble(nibble_t *this, nibble_t *other) {
  return (this->nibble_value & other->nibble_value) == this->nibble_value;
}

// Test the bits (if they are all 1 or not)
_Bool SmolInt_testBits_nibble(nibble_t *this, nibble_t *other) {
  return (this->nibble_value & other->nibble_value) != 0;
}

// The Chunk (of an) Int (15-bit signed integer: 0 to 32K)
// Great for Arduino projects and when you feel an int is
// way too big.
struct chunkint_t {
  // This uses 15 bits exactly out of the standard
  // default of 16. This reduces 64K to 32K (64 / 2 == 32)
  // If you didn't know that this makes it 32K. Congrats,
  // you just learnt (from a comment) Basic Multiplication/Division!
  uint16_t chunkint_value : 15;
};

void SmolInt_initChunkint(chunkint_t *this) { this->chunkint_value = 0; }

chunkint_t *SmolInt_incPre_chunkint(chunkint_t *_this) {
  _this->chunkint_value = (_this->chunkint_value + 1) & 0x7FFF;
  return _this;
}
chunkint_t *SmolInt_incPost_chunkint(chunkint_t *this) {
  chunkint_t *__old = this; // heap cuz why not?
                            // let's do a bit of contradiction.
                            // I recently read a book about chemistry and the
                            // countless exceptions, so... have luck!
  this->chunkint_value = (this->chunkint_value + 1) & 0x7FFF;
  return __old;
}

chunkint_t *SmolInt_decPre_chunkint(chunkint_t *this) {
  this->chunkint_value = (this->chunkint_value - 1) & 0x7FFF;
  return this;
}
chunkint_t *SmolInt_decPost_chunkint(chunkint_t *this) {
  chunkint_t *__old = this; // heap cuz why not?
                            // let's do a bit of contradiction.
                            // I recently read a book about chemistry and the
                            // countless exceptions, so... have luck!
  this->chunkint_value = (this->chunkint_value - 1) & 0x7FFF;
  return __old;
}

void SmolInt_forceBitsOn_chunkint(chunkint_t *this, uint16_t mask) {
  this->chunkint_value = (this->chunkint_value | mask) & 0x7FFF;
}
void SmolInt_forceBitsOff_chunkint(chunkint_t *this, uint16_t mask) {
  this->chunkint_value = (this->chunkint_value & ~mask) & 0x7FFF;
}

chunkint_t *SmolInt_isAssign_chunkint(chunkint_t *this, chunkint_t *other) {
  this->chunkint_value = other->chunkint_value & 0x7FFF;
  return this;
}
_Bool SmolInt_isEquals_chunkint(chunkint_t *this, chunkint_t *other) {
  return (this->chunkint_value & other->chunkint_value) == this->chunkint_value;
}

// Test the bits (if they are all 1 or not)
_Bool SmolInt_testBits_chunkint(chunkint_t *this, chunkint_t *other) {
  return (this->chunkint_value & other->chunkint_value) != 0;
}

// A byte sized flag container
// The True Smol Int (6-bit integer: 0 to 63)
// Leaves 2 bits for hardware status flags inside the same byte!
struct FlaggedByte {
  uint8_t value : 6;   // Range 0 to 63 (Perfect for word counts per line)
                       // This is 4 bits
  _Bool isHovered : 1; // UI State flag 1
  _Bool isFocused : 1; // UI State flag 2
};
// A byte sized flag container
typedef FlaggedByte fByte_t;

struct DoubleByte {
  uint8_t left;
  uint8_t right;
};

typedef DoubleByte dByte_t;
typedef DoubleByte lFData_t;

#ifdef SMOL_INT_LIBRARY_FLAG__USE_SNAME
typedef PackPackedPair8 pByte_t;
typedef DoubleByte lFData_t;
#endif

// #endif // <- headg CLAY
#endif
