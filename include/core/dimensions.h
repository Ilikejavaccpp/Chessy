#pragma once

// include these header guards for older build systems
// or just for 'textbook' people. This is a preventive measure
#ifndef CHESSY_DIMENSIONS_H
#define CHESSY_DIMENSIONS_H

#define CHESSY_WIN_WIDTH 800
#define CHESSY_WIN_HEIGHT 500

// May change shit later
inline const int width = 800;
inline const int height = 500;

// provide a title
inline const char title[7] = "chessy";

// Chessboard Grid Scaling Specifications
inline const int boardSize = 8;
inline int squareSize = 50;

// Absolute Position Alignments (Computed Centers)
// inline const int boardOffsetX = (width - (boardSize * squareSize)) /
//                                 3; // Shifted left to make sidebar space
// Made them non consts so that we can work with them
inline int boardOffsetX = 50; // MAGIC NUMBER >:D
inline int boardOffsetY = (height - (boardSize * squareSize)) / 2;

// Interface Sidebar Elements Sizing Panels
inline int panelWidth = 260;
inline const int padding = 40;

#endif
