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
#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct HexView {
    size_t id;
    char name[1024];
    size_t start, end;
    ImVec4 color;
};

struct HexEdit {
private:
    // used for view selection
    bool m_clicked = false;
    size_t m_click_start, m_click_current;

    size_t m_selected_view = 0;

public:
    // all the current views
    std::vector<HexView> m_views;

    std::string project_path;

    uint8_t* mem_data = NULL;
    size_t mem_size = 0;
    size_t base_display_addr = 0;

    // original memory editor settings
    // todo: refactor
    bool            Open;                                   // = true   // set to false when DrawWindow() was closed. ignore if not using DrawWindow
    bool            ReadOnly;                               // = false  // set to true to disable any editing
    int             Rows;                                   // = 16     //
    bool            OptShowAscii;                           // = true   //
    bool            OptShowHexII;                           // = false  //
    bool            OptGreyOutZeroes;                       // = true   //
    int             OptMidRowsCount;                        // = 8      // set to 0 to disable extra spacing between every mid-rows
    int             OptAddrDigitsCount;                     // = 0      // number of addr digits to display (default calculated based on maximum displayed addr)
    std::function<uint8_t(uint8_t* data, size_t off)> ReadFn;
    std::function<void(uint8_t* data, size_t off, uint8_t d)> WriteFn;

    bool            ContentsWidthChanged;
    char            DataInputBuf[32];
    char            AddrInputBuf[32];
    size_t          GotoAddr;

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

    void LoadFile(const char* path);
    void LoadProject();
    void Save();

    void CalcSizes(Sizes& s);

    // creates everything ( hexedit, view & graph )
    void BeginWindow(const char *title, size_t w, size_t h);

    // renders the content of the hex editor window
    void DrawHexEditContents();
    // renders the content of the hex view window
    void DrawHexViewContents();
};
