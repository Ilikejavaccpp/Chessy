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
#if !defined(CHESSY_BACKEND_C_CORE_ENDEC) &&                                   \
    !defined(CHESSY_BACKEND_C_CORE_CODEC) &&                                   \
    !defined(CHESSY_BACKEND_C_CORE_ENCODEC)

// include the file io header `file_io.h` or the c one
// for the actual definition...
// it is already included so no need to panic
#define CHESSY_BACKEND_C_CORE_CODEC 1
#define CHESSY_BACKEND_C_CORE_ENDEC 1 // file name here

#include <sys/cdefs.h> // for nice stuff and no `#ifdef __cplusplus ...` here and there

// No mangling
__BEGIN_DECLS

#ifndef CHESSY_BACKEND_C_CORE_FILEIO
#include <stdint.h>
#endif

/* Flags categorized as file i/o rather than encryptography (encrypt-decrypt) */

// Change this to what byte size you like (standard practice is 2^n)
// Default is 256 bytes or 2^8 (bytes)
#define CHESSY_BACKEND_C_CORE_FILEIO_FLAG__VBATCH_MAX_SIZE 256
#define CHESSY_BACKEND_C_CORE_FILEIO_FLAG__VBATCH_MAX                          \
  CHESSY_BACKEND_C_CORE_FILEIO_FLAG__VBATCH_MAX_SIZE // Change this to what byte
                                                     // size you like (standard
                                                     // practice is 2^n)
#define CHESSY_BACKEND_FILEIO_FLAG__VBATCH_MAX_SIZE  // Change this to what byte
                                                     // size you like (standard
                                                     // practice is 2^n)

/* A struct holding the move type..
 * This is so that i don't want fwd-impl to bite me.
 * Also, this technically counts as a `core/` utility
 * But is instead defined in an encryption header... question my design choices
 * later. */
enum ChessMoveType {
  // Standard moves: refer to [Chess.com](https://www.chess.com)
  _MOVE_BEST = 3,
  _MOVE_EXCELLENT = 4,
  _MOVE_GOOD = 5,
  _MOVE_BOOK = 6,
  _MOVE_INACCURACY = 7,
  _MOVE_MISTAKE = 9,
  _MOVE_BLUNDER = 10,

  // Miscellaneous
  _MOVE_FORCED = 0, // forced

  _MOVE_MISS = 8,      // misc, to be implemented
  _MOVE_BRILLIANT = 1, // misc, to be implemented
  _MOVE_GREAT = 2,     // misc, to be implemented
};

/* You can tweak with this.. */
struct ChessEngineEvalMetaData {
  int16_t eval_sc;   // the evaulation score measured by stockfish and other
                     // engines in centi pawns
  uint8_t depth;     // the (current) maximum depth
  uint8_t validator; // the validator token (delimeter) used for separating this
                     // meta data from other obfuscated chessy data
};

// C namespacing and shorter convenient names
#if defined(CHESSY_BACKEND_C_CORE_ENDEC_FLAG__USE_SNAME) ||                    \
    defined(CHESSY_BACKEND_C_CORE_CODEC_FLAG__USE_SNAME) ||                    \
    defined(CHESSY_BACKEND_C_CORE_ENDEC_FLAG__USE_NAMESPACE) ||                \
    defined(CHESSY_BACKEND_C_CORE_CODEC_FLAG__USE_NAMESPACE)
typedef struct ChessEngineEvalMetaData ChessEngineEval;
typedef struct ChessEngineEvalMetaData ChessEngineEvalData;
typedef struct ChessEngineEvalMetaData ChessEngineEvalFileData;
#endif

/*
 * Encodes a 64-square board/grid array using RLE and saves it to a file.
 * It basically is a wrapper for a file-i/o utility function called
 * `packSmallBatch(..)`
 *
 * Parameters and their explanations
 *  - `file_path` -> Destination file path (e.g., "saves/slot1.bin")
 *  - `board_64`  -> Pointer to the raw 64-byte chess board array
 */
void encodeMatrix(const char *file_path, const uint8_t *board_64);

/*
 * Encodes and appends a 64-square board/grid array using RLE and saves it to a
 * file. It basically is a wrapper for a file-i/o utility function called
 * `packSmallAppendBatch(..)`
 *
 * Parameters and their explanations
 *  - `file_path` -> Destination file path (e.g., "saves/slot1.bin")
 *  - `board_64`  -> Pointer to the raw 64-byte chess board array
 */
void encodeAppendMatrix(const char *file_path, const uint8_t *board_64);

/*
 * Decodes an RLE binary file back into a 64-square board array.
 *
 * Parameters and their explanations
 *  - `file_path`    -> Source file path
 *  - `out_board_64` -> Target 64-byte array buffer to populate
 */
void decodeMatrix(const char *file_path, uint8_t **out_board_64);

/*
 * Decodes an RLE binary file chunk back into a 64-square board array via
 * indexing.
 *
 * Parameters and their explanations
 *  - `file_path`    -> Source file path
 *  - `out_board_64` -> Target 64-byte array buffer to populate
 *  - `board_index`  -> The section at which the block begins/prev block ends
 */
void decodeMatrixIndexed(const char *file_path, uint8_t **out_board_64,
                         uint32_t board_index);

/*
 * Encodes a text stream (move sequence, fen, etc.) into an obfuscated binary
 * file.
 *
 * Parameters and their explanations
 *  - `file_path`   -> Destination file path (e.g. "data/text.bin")
 *  - `text_stream` -> The stream of contiguous text represented as a string
 * (e.g. "I <heart> chess")
 */
void encodeTextStream(const char *file_path, const char *text_stream);

/*
 * Decodes a text stream (move sequence, fen, etc.) put from an obfuscated
 * binary file into a readable contiguous string
 *
 * Parameters and their explanations
 *  - `file_path`  -> Origin file path (e.g. "data/text.bin")
 *  - `out_buffer` -> Destination stream: The stream to represented as a
 *  string of contiguous text
 *  - `size_max`   -> maximum size of the destination buffer/stream
 */
void decodeTextStream(const char *file_path, int8_t **out_buffer,
                      uint8_t *size_max);

// TODO: for later... v1.0.5 sneek peak
#if defined(CHESSY_BACKEND_C_CORE_ENDEC_FLAG__USE_NAMESPACE) ||                \
    defined(CHESSY_BACKEND_C_CORE_CODEC_FLAG__USE_NAMESPACE)
/* We use snake case here since they classify as `objects` in C++ */

// NOTE: it is tedious to rewrite, and also they are exact copies just mangled
//  manually, so please... use the functions that aren't namespaced

/*
 * Encodes a 64-square board/grid array using RLE and saves it to a file.
 * It basically is a wrapper for a file-i/o utility function called
 * `packSmallBatch(..)`
 *
 * Parameters and their explanations
 *  - `file_path` -> Destination file path (e.g., "saves/slot1.bin")
 *  - `board_64`  -> Pointer to the raw 64-byte chess board array
 */
void codec_encode_matrix(const char *file_path, const uint8_t *board_64);

/*
 * Decodes an RLE binary file back into a 64-square board array.
 *  - `file_path`    -> Source file path
 *  - `out_board_64` -> Target 64-byte array buffer to populate
 */
void codec_decode_matrix(const char *file_path, uint8_t **out_board_64);

#endif

/* Stockfish interaction api.. encrypt decrypt for faster stuff.
 * STATUS: Currently, gonna make some declarations and implementations (no child
 * process spawn & management yet) for the next 2—3 days --> Jun 23 2026, Tue ==
 * TODAY */

/* Fen stuff: aka non stockfish */

/*
 * HELPER
 *
 * Converts a standard integer chess board layout (chessy board) into a fen
 * string
 */
void convert_boardToFen(char **__restrict out_fen_string,
                        uint8_t *__restrict board_64);

/*
 * HELPER
 *
 * Converts a standard fen string into a standard integer chess board (chessy
 * board) layout.
 *
 * Please, pass EXACTLY 64 bits into this since it is using a non-dynamic memset
 * for speed and predictability.
 */
void convert_fenToBoard(char *__restrict fen_string,
                        uint8_t **__restrict out_board_64);

/*
 * Encodes a fen string into an obfuscated file. Useful for saving a position
 * since it is pretty much universal. (This is a custom non wrapper function)
 *
 * Parameters and their explanations
 *  - `file_path`  -> the destination file path
 *  - `fen_string` -> the fen string to be encoded.
 */
void encodeFen(const char *file_path, int8_t *fen_string);

/*
 * Decodes a fen string from an obfuscated file (fen binary file).
 *
 * Parameters and their explanations
 *  - `file_path` -> the source/origin path of the file containing the fen
 * strings.
 *  - `out_fen`   -> the output (aka the fen string) is gonna be stored here
 */
void decodeFen(const char *file_path, int8_t **out_fen);

/* Stockfish stuff */

/*
 * Encodes a string of engine lines of depth `depth` into an obfuscated file..
 *
 * ----------------------------------------------------------------------------+
 * Brief:                                                                      |
 * So basically.. everytime it encounters a newline... it parses that          |
 * differently as a different batch or mini batch (via a mini string: see      |
 * `file_io.h`)                                                                |
 *  It at the end uses a delimeter to signify the depth (so that we don't have |
 * to do codebase littering).                                                  |
 * ----------------------------------------------------------------------------+
 *
 * Parameters and their explanations
 *  `file_path`    -> the destination path of the file to be encoded in
 *  `engine_lines` -> the engine lines (usually top 3 or top 5<-CHUNKY) in UCI
 *  `depth`        -> maximum depth for stockfish engine evaluation lines
 *  `eval_score`   -> the score evaluated in centipawns
 */
void encodeEngineLines(const char *file_path, int8_t *engine_lines,
                       uint8_t depth, uint16_t eval_score);

/*
 * Decodes the engine lines from an obfuscated file. Looks for a stockfish
 * (engine) section
 *
 * ---------------------------------------------------------------------------+
 * Note to the user who recieved this program:                                |
 *                                                                            |
 * NOTE: Please, just do what you want, this is safe and just tweak the depth |
 * since the file already tracks count of the depth.. (FILE LEVEL SAFETY!!).  |
 * Also the depth parameter acts as both a                                    |
 *  1. variable for depth (main)                                              |
 *  2. error logging variable (sub)                                           |
 * ---------------------------------------------------------------------------+
 *
 * Parameters and their explanations
 *  `file_path`    -> the source/origin path of the file to be decoded from
 *  `engine_lines` -> the engine lines (usually top 3 or top 5<-CHUNKY) in UCI
 * (so please use `uint8_t` as the index & size (ln. count))
 *  `depth`        ->
 * maximum depth for stockfish engine evaluation lines (again, pls use
 * `uint8_t`)
 *  `eval_scores`   -> the scores evaluated in centipawns
 */
void decodeEngineLines(const char *file_path, int8_t **__restrict out_lines,
                       uint8_t *__restrict depth,
                       uint16_t **__restrict eval_scores);

/* For converting to UCI (stockfish/engine language) or to PGN (normal human
 * language) */

/* ... Lorem ipsum . Hello world DUMMY */
void convert_pgnToUCI(int8_t *__restrict *out_uci,
                      int8_t *__restrict pgn_lines);

/* ... Lorem ipsum . Hello world DUMMY */
void convert_uciToPGN(int8_t *__restrict *out_pgn,
                      int8_t *__restrict uci_lines);

__END_DECLS

#endif
