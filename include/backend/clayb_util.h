#pragma once

#ifndef CHESSY_BACKEND_C_UI_LAYOUT
#define CHESSY_BACKEND_C_UI_LAYOUT

#ifdef __cplusplus
extern "C" {
#endif

#include "clay.h"

void InitChessyClayArena(Clay_Arena *arena, void *memoryBuffer, size_t size,
                         Clay_ErrorHandler *handler, int width, int height);
float BuildChessyMenuAboutPage(float viewportW, float viewportH,
                               float estimatedContentHeight,
                               float rl_deltaTime);
float BuildChessyMenuHelpPage(float viewportW, float viewportH,
                              float estimatedContentHeight, float rl_deltaTime);

#ifdef __cplusplus
}
#endif

#endif // CHESSY_BACKEND_C_UI_LAYOUT
