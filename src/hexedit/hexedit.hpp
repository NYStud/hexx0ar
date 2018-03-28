//
// Todo/Bugs:
// - Arrows are being sent to the InputText() about to disappear which for LeftArrow makes the text cursor appear at position 1 for one frame.

#pragma once
#include <stdio.h>
#include <utility>
#include <iostream>
#include "imgui.h"
#include <vector>
#include <tuple>

struct HexEdit
{
private:
public:
  bool            Open;                                   // = true   // set to false when DrawWindow() was closed. ignore if not using DrawWindow
  bool            ReadOnly;                               // = false  // set to true to disable any editing
  int             Rows;                                   // = 16     //
  bool            OptShowAscii;                           // = true   //
  bool            OptShowHexII;                           // = false  //
  bool            OptGreyOutZeroes;                       // = true   //
  int             OptMidRowsCount;                        // = 8      // set to 0 to disable extra spacing between every mid-rows
  int             OptAddrDigitsCount;                     // = 0      // number of addr digits to display (default calculated based on maximum displayed addr)
  uint8_t              (*ReadFn)(uint8_t* data, size_t off);        // = NULL   // optional handler to read uint8_ts
  void            (*WriteFn)(uint8_t* data, size_t off, uint8_t d); // = NULL   // optional handler to write uint8_ts

  bool            ContentsWidthChanged;
  size_t          DataEditingAddr;
  bool            DataEditingTakeFocus;
  char            DataInputBuf[32];
  char            AddrInputBuf[32];
  size_t          GotoAddr;

  std::vector<std::tuple<size_t, size_t, ImU32>> Highlights;

  bool Clicked = false;
  size_t ClickStartPos, ClickCurrentPos;

  HexEdit();

  void GotoAddrAndHighlight(size_t addr_min, size_t addr_max)
  {
    GotoAddr = addr_min;
    //HighlightMin = addr_min;
    //HighlightMax = addr_max;
  }

  struct Sizes
  {
    int     AddrDigitsCount;
    float   LineHeight;
    float   GlyphWidth;
    float   HexCellWidth;
    float   SpacingBetweenMidRows;
    float   PosHexStart;
    float   PosHexEnd;
    float   PosAsciiStart;
    float   PosAsciiEnd;
    float   WindowWidth;
  };

  void CalcSizes(Sizes& s, size_t mem_size, size_t base_display_addr);

  // Standalone Memory Editor window
  void DrawWindow(const char* title, uint8_t* mem_data, size_t mem_size, size_t base_display_addr = 0x0000);

  // Memory Editor contents only
  void DrawContents(uint8_t* mem_data, size_t mem_size, size_t base_display_addr = 0x0000);
};
