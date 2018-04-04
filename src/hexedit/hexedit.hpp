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
#include <functional>
#include "json.hpp"
#include "opengl/glclasses.hpp"

using json = nlohmann::json;

enum HexViewMode { HexViewMode_Filled, HexViewMode_Line };

struct HexView {
  size_t id;
  char name[1024];
  size_t start, end;
  HexViewMode mode = HexViewMode_Filled;
  ImVec4 color;
};

inline bool operator< (const HexView& lhs, const HexView& rhs){ return lhs.start < rhs.start; }
inline bool operator> (const HexView& lhs, const HexView& rhs){ return rhs < lhs; }
inline bool operator<=(const HexView& lhs, const HexView& rhs){ return !(lhs > rhs); }
inline bool operator>=(const HexView& lhs, const HexView& rhs){ return !(lhs < rhs); }

struct HexEdit {
private:
  // current width/height
  size_t m_width = 0;
  size_t m_height = 0;

  // current sizes
  uint32_t AddrDigitsCount;
  float LineHeight;
  float GlyphWidth;
  float HexCellWidth;
  float SpacingBetweenMidColumns;
  float PosHexStart;
  float PosHexEnd;
  float PosAsciiStart;
  float PosAsciiEnd;
  float HexEdit_WindowWidth;
  float HexView_WindowWidth;
  float HexGraph_WindowWidth;

  // used for view selection
  bool m_clicked = false;
  size_t m_click_start, m_click_current;
  size_t m_selected_view = 0;
  int m_current_view = -1;
  size_t m_cursor = 0;

  std::vector<float> data_X;
  std::vector<float> data_Y;
  std::vector<float> data;

  size_t getRow(size_t addr);
  size_t getCol(size_t addr);
  // returns upper left x
  float getTopX(size_t addr);
  // returns upper left y
  float getTopY(size_t addr);
  // returns lower right x
  float getBottomX(size_t addr);
  // returns lower right y
  float getBottomY(size_t addr);

public:
  // all the current views
  std::vector<HexView> m_views;

  // path for storing views
  std::string project_path;

  // data
  uint8_t* mem_data = NULL;
  size_t mem_size = 0;
  size_t base_display_addr = 0;

  // original memory editor settings
  // todo: refactor
  bool            Open;               // set to false when DrawWindow() was closed. ignore if not using DrawWindow
  bool            ReadOnly;           // set to true to disable any editing
  int             Columns;            //
  bool            OptShowAscii;       //
  bool            OptGreyOutZeroes;   //
  int             OptMidColumnsCount; // set to 0 to disable extra spacing between every mid-rows
  int             OptAddrDigitsCount; // number of addr digits to display (default calculated based on maximum displayed addr)

  std::function<uint8_t(uint8_t* data, size_t off)> ReadFn;
  std::function<void(uint8_t* data, size_t off, uint8_t d)> WriteFn;

  bool            ContentsWidthChanged;
  char            DataInputBuf[32];
  char            AddrInputBuf[32];
  size_t          GotoAddr;

  HexEdit();

  void LoadFile(const char* path);
  void LoadProject();
  void Save();

  void CalcSizes();

  // creates everything ( hexedit, view & graph )
  void BeginWindow(const char *title, size_t w, size_t h, size_t m_delta);

  void DrawRightClickPopup();

  // renders the content of the hex editor window
  void DrawHexEdit();
  // renders the content of the hex view window
  void DrawHexView();
  // render the graph
  void DrawHexGraph();
  // render the table
  void DrawHexTable();
};
