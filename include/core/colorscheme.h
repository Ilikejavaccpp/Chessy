#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <raylib.h>
#include <string>
#include <unordered_map>

#ifndef CHESSY_COLORSCHEME_H
#define CHESSY_COLORSCHEME_H

// Colorschemes: predefined
// Format: { red, green, blue, alpha }

// Board
#define CHESSY_COLOR_LIGHT_BOARDSQ Color{240, 217, 181, 255} // Custom tan
#define CHESSY_COLOR_DARK_BOARDSQ Color{181, 136, 99, 255}   // Custom brown

#define CHESSY_COLOR_LIGHT_BOARDSQ_CAPTURED Color{230, 212, 180, 255}
#define CHESSY_COLOR_DARK_BOARDSQ_CAPTURED Color{177, 134, 98, 255}

#define CHESSY_COLOR_SEMI_SELECTED Color{255, 0, 0, 100} // 100/255 transparency
#define CHESSY_COLOR_LIGHT_CHECKEDK                                            \
  Color{231, 76, 60, 130} // When the king is in check

// Tints
#define CHESSY_COLOR_TINT_HOVER Color{23, 147, 209, 60}
#define CHESSY_COLOR_TINT_SELECTED Color{46, 204, 113, 60}

// Background
#define CHESSY_COLOR_DARK_BACKGROUND Color{40, 44, 52, 255}

// Background for the menu header
#define CHESSY_COLOR_DARK_BACKGROUND_MENU_HEADER                               \
  Color{50, 47, 51, 180} // make it less transparent
                         // so that it stands out

// Background for the menu body
#define CHESSY_COLOR_DARK_BACKGROUND_MENU_BODY ColorAlpha(BLACK, 0.3f)

// Foreground
#define CHESSY_COLOR_DARK_FOREGROUND Color{230, 230, 167, 255}

inline std::map<std::string, Color> GetPinkFloydPalette() {
  return {
      // Core Board Canvas
      {"void_black", Color{11, 12, 16, 255}},      // #0B0C10
      {"obsidian_dark", Color{31, 40, 51, 255}},   // #1F2833
      {"prism_silver", Color{197, 198, 199, 255}}, // #C5C6C7
      {"pure_beam", Color{244, 246, 249, 255}},    // #F4F6F9)

      // The Refracted Rainbow (UI accents and active squares)
      {"breathe_red", Color{255, 0, 92, 255}}, // Spectrum start
      {"time_orange", Color{255, 133, 0, 255}},
      {"money_green", Color{0, 255, 140, 255}},
      {"eclipse_blue", Color{0, 180, 255, 255}} // Spectrum end
  };
}

class Colorscheme {
private:
  std::unordered_map<std::string, Color> data;
  uint16_t length; // 128 Colors all accessible via a dictionary lookup

public:
  // Initializes the default colorscheme if no params
  // TODO: will implement a colorscheme parameter
  void init() {
    // Set the background color
    append_color("background_dark", CHESSY_COLOR_DARK_BACKGROUND);
    append_color(
        "background_dark_menu_header",
        CHESSY_COLOR_DARK_BACKGROUND_MENU_HEADER); // for the header of the
                                                   // default start menu
    append_color("background_dark_menu_body",
                 CHESSY_COLOR_DARK_BACKGROUND_MENU_BODY);

    // Set the foreground color of text
    append_color("foreground_dark", CHESSY_COLOR_DARK_FOREGROUND);

    // Set the color of the chess squares
    append_color("square_light", CHESSY_COLOR_LIGHT_BOARDSQ);
    append_color("square_dark", CHESSY_COLOR_DARK_BOARDSQ);

    append_color("square_cap_light", CHESSY_COLOR_LIGHT_BOARDSQ_CAPTURED);
    append_color("square_cap_dark", CHESSY_COLOR_DARK_BOARDSQ_CAPTURED);

    // Set the background color (square) of a selected piece.
    append_color("selected", CHESSY_COLOR_SEMI_SELECTED);

    // Set the background color (square) of the king when in check.
    append_color("check_square", CHESSY_COLOR_LIGHT_CHECKEDK);

    // Tints
    append_color("hover_tint",
                 CHESSY_COLOR_TINT_HOVER); // Translucent aqua/cyan tint layer
                                           // overlay for hovering tiles
    append_color(
        "selected_tint",
        CHESSY_COLOR_TINT_SELECTED); // Translucent soft green or gold highlight
                                     // overlay for selected pieces
    append_color("dot_tint",
                 Color{0, 0, 0, 50}); // Translucent dark dot for empty tiles
    append_color(
        "ring_tint",
        Color{23, 147, 209, 100}); // Translucent accent ring for captures

    // Set the accent for border
    append_color("accent", Color{23, 147, 209, 130});
  }

  // Adds a color to the back row.
  // NOTE: we don't need pass by ref since it just ^C^V's the tuple ({ ... })
  //       and pushes it to the data.
  void append_color(std::string key, Color color) {
    data.insert_or_assign(key, color);
  }

  // Removes a color via index. Default index is back hence
  // with no parameters, it just removes/clears the color at the back of the
  // stack (LastInFirstOut)
  void remove_color(std::string &key) { data.erase(key); }

  /**
   * Core overloads and methods
   **/
  // Overload for indexing
  // OVERLOAD: Overloading the operator allows dicitonary-like lookups e.g.,
  // palette['background']
  Color operator[](const std::string &key) const {
    // Find the element to avoid accidentally injecting empty keys via
    // operator[]
    auto it = data.find(key);
    if (it != data.end()) {
      return it->second;
    }

    // Critical logging fallback alert if a typo happens during development
    std::cout << "[WARNING] : Color key \"" << key
              << "\" not found! Falling back to MAGENTA.\n";
    return MAGENTA;
  }

  // If you don't want indexing... this function acts as the same thing
  Color getColor(const std::string &key) const {
    // Find the element to avoid accidentally injecting empty keys via
    // operator[]
    auto it = data.find(key);
    if (it != data.end()) {
      return it->second;
    }

    // Critical logging fallback alert if a typo happens during development
    std::cout << "[WARNING] : Color key \"" << key
              << "\" not found! Falling back to obnoxious MAGENTA.\n";
    return MAGENTA;
  }

  // Size
  unsigned long getSize() { return this->data.size(); }
};

#endif
