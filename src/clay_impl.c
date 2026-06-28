// Clay Implementation dedicated C++ source file
// -- the chessy project
//
// Any helper functions you write here or was written here will have native,
// unmangled C linkage names
#define CLAY_IMPLEMENTATION
#include "clay.h"

void InitChessyClayArena(Clay_Arena *arena, void *memoryBuffer, size_t size,
                         Clay_ErrorHandler *handler, int width, int height) {
  *arena = Clay_CreateArenaWithCapacityAndMemory(size, memoryBuffer);
  Clay_Initialize(*arena, (Clay_Dimensions){width, height}, *handler);
}

/*
 * Please pass the delta time parameter over here via raylib's
 * `GetFrameTime()` function.
 */
float BuildChessyMenuAboutPage(float viewportW, float viewportH,
                               float estimatedContentHeight,
                               float rl_deltaTime) {
  // Set this for resize
  Clay_SetLayoutDimensions((Clay_Dimensions){viewportW, viewportH});
  Clay_BeginLayout();

  // 1. Root bounding canvas box
  CLAY(CLAY_ID("ScrollWindowParent"),
       {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .sizing = {CLAY_SIZING_FIXED(viewportW),
                              CLAY_SIZING_FIXED(viewportH)}}}) {
    // 2. THE SCROLL CONTAINER: Manages mouse wheel steps and clips children
    CLAY(CLAY_ID("TextBoxScrollRegion"),
         {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}},
          // Put scroll here
          .clip = {.horizontal = false,
                   .vertical = true,
                   .childOffset = Clay_GetScrollOffset()}}) {
      // 3. VIRTUAL FILLER ELEMENT: Dictates how long the scrolling field is
      CLAY(
          CLAY_ID("VirtualTextHeightBlock"),
          {.layout = {.sizing = {CLAY_SIZING_GROW(),
                                 CLAY_SIZING_FIXED(estimatedContentHeight)}}}) {
      }
    }
  }
  Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(
      Clay_GetElementId(CLAY_STRING("TextBoxScrollRegion")));

  float currentYOffset = 0.0f;
  if (scrollData.found) {
    currentYOffset = scrollData.scrollPosition->y;
  }

  // Finalize layout rendering metrics
  Clay_EndLayout(rl_deltaTime);

  return currentYOffset; // return the calculated offset.
}

// The rl is either
// - RayLib or
// - ReaL
float BuildChessyMenuHelpPage(float viewportW, float viewportH,
                              float estimatedContentHeight,
                              float rl_deltaTime) {
  // Set this for resize
  Clay_SetLayoutDimensions((Clay_Dimensions){viewportW, viewportH});

  // Gonna put some flags
  Clay_BeginLayout();

  // 1. Root bounding canvas box
  CLAY(CLAY_ID("ScrollWindowParent"),
       {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .sizing = {CLAY_SIZING_FIXED(viewportW),
                              CLAY_SIZING_FIXED(viewportH)}}}) {
    // 2. THE SCROLL CONTAINER: Manages mouse wheel steps and clips children
    CLAY(CLAY_ID("TextBoxScrollRegion"),
         {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}},
          // Put scroll here
          .clip = {.horizontal = false,
                   .vertical = true,
                   .childOffset = Clay_GetScrollOffset()}}) {
      // 3. VIRTUAL FILLER ELEMENT: Dictates how long the scrolling field is
      CLAY(
          CLAY_ID("VirtualTextHeightBlock"),
          {.layout = {.sizing = {CLAY_SIZING_GROW(),
                                 CLAY_SIZING_FIXED(estimatedContentHeight)}}}) {
      }
    }
  }
  Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(
      Clay_GetElementId(CLAY_STRING("TextBoxScrollRegion")));

  float currentYOffset = 0.0f;
  if (scrollData.found) {
    currentYOffset = scrollData.scrollPosition->y;
  }

  // Finalize layout rendering metrics
  Clay_EndLayout(rl_deltaTime);

  return currentYOffset; // return the calculated offset.
}

// The rl is either
// - RayLib or
// - ReaL
float BuildChessyMenuNewsPage(float viewportW, float viewportH,
                              float estimatedContentHeight,
                              float rl_deltaTime) {
  Clay_BeginLayout();

  // If you want to put stuff inside with raylib (first put `#include
  // <raylib.h>` and it also must be C code) in this file... Copy this block and
  // add/change some stuff...
  //
  // CLAY(CLAY_ID("NewsPanelCanvas"),
  //      {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
  //                  .sizing = {CLAY_SIZING_FIXED(viewportW),
  //                             CLAY_SIZING_FIXED(viewportH)}},
  //       .clip = {.horizontal = false,
  //                .vertical = true,
  //                .childOffset = Clay_GetScrollOffset()}}) {
  //   CLAY(CLAY_ID("NewsVirtualSpacer"),
  //        {.layout = {.sizing = {CLAY_SIZING_GROW(),
  //                               CLAY_SIZING_FIXED(estimatedContentHeight)}}})
  //                               {}
  // }

  // 1. Root bounding canvas box
  CLAY(CLAY_ID("ScrollWindowParent"),
       {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                   .sizing = {CLAY_SIZING_FIXED(viewportW),
                              CLAY_SIZING_FIXED(viewportH)}}}) {
    // 2. THE SCROLL CONTAINER: Manages mouse wheel steps and clips children
    CLAY(CLAY_ID("TextBoxScrollRegion"),
         {.layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                     .sizing = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}},
          // Put scroll here
          .clip = {.horizontal = false,
                   .vertical = true,
                   .childOffset = Clay_GetScrollOffset()}}) {
      // 3. VIRTUAL FILLER ELEMENT: Dictates how long the scrolling field is
      CLAY(
          CLAY_ID("VirtualTextHeightBlock"),
          {.layout = {.sizing = {CLAY_SIZING_GROW(),
                                 CLAY_SIZING_FIXED(estimatedContentHeight)}}}) {
      }
    }
  }

  Clay_ScrollContainerData scrollData = Clay_GetScrollContainerData(
      Clay_GetElementId(CLAY_STRING("TextBoxScrollRegion")));

  float currentYOffset = 0.0f;
  if (scrollData.found) {
    currentYOffset = scrollData.scrollPosition->y;
  }

  Clay_EndLayout(rl_deltaTime);

  return currentYOffset;
}
/*
here's my implementation...

src/clay_impl.c (keep things organized low-level):

```c


```
 */
