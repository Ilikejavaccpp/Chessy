#pragma once

// May modify these 3 lines for C/C++ support (namespace + C-styled shenanigans)
// Change the line no. to the current line
#ifdef SMOL_INT_LIBRARY_HEADER_C
#error "Compilation error: Included the smol int library twice!\n \
        In included file `smolInt.hpp`...\n \
        In line 6-8: `#ifndef SMOL_INT_LIB...` \n \
        Fix: Please remove the included file *once* or remove/modify \
        this text directly in `smolInt.hpp` \n \
        \n \
        [ERROR] : Included the library twice.\n \
        [FIX]   : Remove the included `smolInt.hpp` or modify this.\n"
#endif

// Guards/Flags for toggling:
// `SMOL_INT_LIBRARY_FLAG__USE_SNAME` -> for shorter spellings (aliases)
// `SMOL_INT_LIBRARY_FLAG__USE_ENGSPELL` -> for correct spelling (aliases)

#ifndef SMOL_INT_LIBRARY_HEADER_CPP
#define SMOL_INT_LIBRARY_HEADER_CPP

#define SMOL_INT_LIBRARY__CAST(n)                                              \
  (*(unsigned char *)&(n)) // casts a smol int type to an int.

#include <cstdint>

namespace SmolInt {

// A packed pair of integers sharing ONE single byte.
// Perfect for layout states, UI grid coordinates, or paragraph flags.
struct PackedPair8 {
  uint8_t left : 4;  // 4 bits: range 0 to 15
  uint8_t right : 4; // 4 bits: range 0 to 15
};

// The Nibble Int (4-bit integer: 0 to 15)
// Takes up 1 byte standalone due to alignment, but array-packable!
class nibble_t {
private:
  uint8_t value : 4;

public:
  nibble_t() : value(0) {}
  nibble_t(uint8_t val)
      : value(val & 0x0F) {} // Mask out overflow automatically

  // Implicitly drop straight into calculations like a normal integer
  operator uint8_t() const { return value; }

  // Operator Overloads to make it feel like a real native type
  nibble_t &operator=(uint8_t val) {
    value = val & 0x0F;
    return *this;
  }

  nibble_t &operator++() { // Prefix increment
    value = (value + 1) & 0x0F;
    return *this;
  }
  nibble_t operator++(int) { // Post fix increment
    nibble_t __old = *this;
    value = (value + 1) & 0x0F;
    return __old;
  }

  nibble_t &operator--() { // Prefix decrement
    value = (value - 1) & 0x0F;
    return *this;
  }
  nibble_t operator--(int) { // Post fix decrement
    nibble_t __old = *this;
    value = (value - 1) & 0x0F;
    return __old;
  }
};

// The Chunk (of an) Int (15-bit signed integer: 0 to 32K)
// Great for Arduino projects and when you feel an int is
// way too big.
class chunkint_t {
private:
  // This uses 15 bits exactly out of the standard
  // default of 16. This reduces 64K to 32K (64 / 2 == 32)
  // If you didn't know that this makes it 32K. Congrats,
  // you just learnt (from a comment) Basic Multiplication/Division!
  uint16_t value : 15;

public:
  // Implement overloads here.
  // Btw, We can just like do some functions to toggle mask on/off for
  // efficiency, so yeah let's do it. Made it a bit friendly.
  chunkint_t() : value(0) {}
  chunkint_t(uint16_t val)
      : value(val & 0x7FFF) {} // 0x7FFF masks exactly 15 bits

  operator uint16_t() const { return value; }

  chunkint_t &operator=(uint16_t val) {
    value = val & 0x7FFF;
    return *this;
  }

  chunkint_t &operator++() {
    value = (value + 1) & 0x7FFF;
    return *this;
  }
  chunkint_t operator++(int) {
    chunkint_t __old = *this;
    value = (value + 1) & 0x7FFF;
    return __old;
  }

  chunkint_t &operator--() {
    value = (value - 1) & 0x7FFF;
    return *this;
  }
  chunkint_t operator--(int) {
    chunkint_t __old = *this;
    value = (value - 1) & 0x7FFF;
    return __old;
  }

  // NEW naming convention!
  bool operator==(nibble_t other) const { return (value & other) == value; }

  // Hardware accelerations (Toggles)
  inline void forceBitsOn(uint16_t mask) { value = (value | mask) & 0x7FFF; }
  inline void forceBitsOff(uint16_t mask) { value = (value & ~mask) & 0x7FFF; }

  // Test the bits (if they are all 1 or not)
  inline bool testBits(uint16_t mask) const { return (value & mask) != 0; }
};

// The True Smol Int (6-bit integer: 0 to 63)
// Leaves 2 bits for hardware status flags inside the same byte!
struct FlaggedByte {
  uint8_t value : 6;  // Range 0 to 63 (Perfect for word counts per line) or for
                      // a chessboard. This is 4 bits
  bool isHovered : 1; // UI State flag 1, the first bit (actually bit no.7)
  bool isFocused : 1; // UI State flag 2, the second bit (actually bit no.8)
};

/* Typedefs
 *
 * FlaggedByte typedef */
typedef FlaggedByte fByte_t;
typedef PackedPair8 packByte_t;

struct DoubleByte {
  uint8_t left;
  uint8_t right;
};

typedef DoubleByte dByte_t;
typedef DoubleByte longFileData_t;

#ifdef SMOL_INT_LIBRARY_FLAG__USE_SNAME
typedef PackPackedPair8 pByte_t;
typedef DoubleByte lFData_t;
// Basically an alias but helps for new people (who think that
// characters aren't == to an integer)
typedef uint_fast8_t bite;
#endif

} // namespace SmolInt

// for those people who hate meme related english spellings
#ifdef SMOL_INT_LIBRARY_FLAG__USE_ENGSPELL
using SmallInt = SmolInt;
#endif

#endif
