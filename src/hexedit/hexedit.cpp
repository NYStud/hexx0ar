#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <application/log.hpp>
#include <algorithm>
#include <SDL2/SDL_video.h>
#include "hexedit.hpp"


static int a = 3;
static int a_e = 20;

namespace fs = boost::filesystem;

void to_json(json& j, const HexView& v) {
  j = json{{"name", v.name},
           {"start", v.start},
           {"end", v.end},
           {"color_r", v.color.x},
           {"color_g", v.color.y},
           {"color_b", v.color.z},
           {"color_a", v.color.w}};
}

void from_json(const json& j, HexView& v) {
  std::string name = j.at("name").get<std::string>();
  if(name.size() > sizeof(v.name)-1)
    name.resize(sizeof(v.name)-1);
  strcpy(v.name, name.data());

  v.start = j.at("start").get<size_t>();
  v.end = j.at("end").get<size_t>();

  v.color.x = j.at("color_r").get<float>();
  v.color.y = j.at("color_g").get<float>();
  v.color.z = j.at("color_b").get<float>();
  v.color.w = j.at("color_a").get<float>();
}

size_t HexEdit::getRow(size_t addr) {
  return (size_t)(addr/Columns);
}

size_t HexEdit::getCol(size_t addr) {
  return addr%Columns;
}

float HexEdit::getTopX(size_t addr) {
  return HexCellWidth*getCol(addr) + (0.5f*GlyphWidth)*((size_t)(getCol(addr)/8));
}

float HexEdit::getTopY(size_t addr) {
  return LineHeight*getRow(addr);
}

float HexEdit::getBottomX(size_t addr) {
  return getTopX(addr)+(2*GlyphWidth);
}
float HexEdit::getBottomY(size_t addr) {
  return getTopY(addr)+LineHeight;
}

int HexEdit::isHighlighted(size_t addr) {
  for(int i = 0; (size_t)i < m_views.size(); i++) {
    if(m_views[i].start <= addr && m_views[i].end >= addr) {
      return i;
    }
  }
  return -1;
}

HexEdit::HexEdit() {
  // Settings
  Open = true;
  ReadOnly = false;
  Columns = 16;
  OptShowAscii = true;
  OptGreyOutZeroes = true;
  OptMidColumnsCount = 8;
  OptAddrDigitsCount = 0;
  ReadFn = [](uint8_t* data, size_t off) -> uint8_t { return (data != 0) ? data[off] : 0; };
  // todo: writefn

  // State/Internals
  ContentsWidthChanged = false;
  memset(DataInputBuf, 0, sizeof(DataInputBuf));
  memset(AddrInputBuf, 0, sizeof(AddrInputBuf));
  GotoAddr = (size_t)-1;

// test data
  for(int x = 0; x < 4; x++) {
    data_X.push_back(x);
  }
  for(int y = 0; y < 6; y++) {
    data_Y.push_back(y);
  }

  for(auto x : data_X) {
    for(auto y : data_Y) {
      data.push_back(x+y);
    }
  }
}

void HexEdit::LoadFile(const char* path) {
  if(fs::exists(fs::path(path))) {
    std::ifstream f(path, std::ios::binary);

    auto fsize = f.tellg();
    f.seekg(0, std::ios::end);
    fsize = f.tellg() - fsize;

    f.seekg(0, std::ios::beg);

    if(mem_data) {
      free(mem_data);
    }

    mem_data = (uint8_t*)malloc(fsize);

    mem_size = fsize;

    f.read((char*)mem_data, fsize);
  }
}

void HexEdit::LoadProject() {
  if(fs::exists(fs::path(project_path))) {
    std::ifstream f(project_path);
    json j;
    f >> j;

    m_views.clear();
    for(auto& element : j["views"]) {
      m_views.push_back(element);
    }
  }
}

void HexEdit::Save() {
  std::ofstream f(project_path);
  json j;

  j["views"] = m_views;

  f << j;
}

void HexEdit::CalcSizes() {
  ImGuiStyle& style = ImGui::GetStyle();
  AddrDigitsCount = OptAddrDigitsCount;
  if (AddrDigitsCount == 0)
    for (size_t n = base_display_addr + mem_size - 1; n > 0; n >>= 4)
      AddrDigitsCount++;
  LineHeight = (float)(int)ImGui::GetTextLineHeight();
  GlyphWidth = ImGui::CalcTextSize("F").x + 1;                  // We assume the font is mono-space
  HexCellWidth = (GlyphWidth * 2.5f);             // "FF " we include trailing space in the width to easily catch clicks everywhere
  SpacingBetweenMidColumns = (HexCellWidth * 0.25f); // Every OptMidColumnsCount columns we add a bit of extra spacing
  PosHexStart = (AddrDigitsCount + 2) * GlyphWidth;
  PosHexEnd = PosHexStart + (HexCellWidth * Columns);
  PosAsciiStart = PosAsciiEnd = PosHexEnd;
  if (OptShowAscii)
  {
    PosAsciiStart = PosHexEnd + GlyphWidth * 1;
    if (OptMidColumnsCount > 0)
      PosAsciiStart += ((Columns + OptMidColumnsCount - 1) / OptMidColumnsCount) * SpacingBetweenMidColumns;
    PosAsciiEnd = PosAsciiStart + Columns * GlyphWidth;
  }
  HexEdit_WindowWidth = PosAsciiEnd + style.ScrollbarSize + style.WindowPadding.x * 2 + GlyphWidth;
  HexView_WindowWidth = (m_width - HexEdit_WindowWidth)/2;
  HexGraph_WindowWidth = (m_width - HexEdit_WindowWidth)/2;
}

void HexEdit::BeginWindow(const char *title, size_t w, size_t h, size_t m_delta) {
  m_width = w;
  m_height = h;

  if(!w || !h)
    return;

  CalcSizes();
  ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(HexEdit_WindowWidth, FLT_MAX));

  Open = true;
  if (ImGui::Begin(title, &Open,ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoTitleBar|
                                ImGuiWindowFlags_NoScrollbar))
  {
    /* menu bar and file handling */
    bool file_open_dialog = false;
    if (ImGui::BeginMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem("Open")) {
          file_open_dialog = true;
        }

        if(ImGui::MenuItem("Save Project")) {
          Save();
        }

        if(ImGui::MenuItem("Load Project")) {
          LoadProject();
        }

        if (ImGui::MenuItem("Close")) {
          if(mem_data)
            free(mem_data);
          mem_data = NULL;
          mem_size = 0;

          m_views.clear();
        }
        if (ImGui::MenuItem("Quit")) {
          exit(0);
        }
        ImGui::EndMenu();
      }
      // Options menu
      if (ImGui::BeginMenu("Options"))
      {
        ImGui::PushItemWidth(56);
        if (ImGui::DragInt("##rows", &Columns, 0.2f, 4, 32, "%.0f rows")) ContentsWidthChanged = true;
        ImGui::PopItemWidth();

        if (ImGui::Checkbox("Show Ascii", &OptShowAscii)) ContentsWidthChanged = true;
        ImGui::Checkbox("Grey out zeroes", &OptGreyOutZeroes);

        ImGui::EndMenu();
      }

      ImGui::EndMenuBar();
    }

    if (file_open_dialog)
      ImGui::OpenPopup("Open File");
    if (ImGui::BeginPopupModal("Open File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
      static char path[4096];
      ImGui::Text("input file path\n\n");
      ImGui::Separator();
      ImGui::InputText("##path", path, sizeof(path));

      if (ImGui::Button("OK", ImVec2(120,0))) {
        LoadFile(path);

        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Button("Cancel", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); }

      ImGui::EndPopup();
    }

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(1))
      ImGui::OpenPopup("context");
    DrawHexEdit();
    if (ContentsWidthChanged)
    {
      CalcSizes();
      ImGui::SetWindowSize(ImVec2(HexEdit_WindowWidth, ImGui::GetWindowSize().y));
    }
  }
  ImGui::SetWindowPos(ImVec2(0,0));
  ImGui::SetWindowSize(ImVec2(HexEdit_WindowWidth, h));

  ImGui::End();

  ImGui::Begin("View", NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_ResizeFromAnySide|
                             ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar);

  ImGui::SetWindowPos(ImVec2(HexEdit_WindowWidth,0));
  ImGui::SetWindowSize(ImVec2(HexView_WindowWidth, h/2));

  DrawHexView();
  ImGui::End();

  ImGui::Begin("Graph", NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_ResizeFromAnySide|
                              ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar);
  ImGui::SetWindowPos(ImVec2((HexEdit_WindowWidth + HexView_WindowWidth),0));
  ImGui::SetWindowSize(ImVec2(HexGraph_WindowWidth, h/2));

  DrawHexGraph();
  ImGui::End();

  ImGui::Begin("Table", NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_ResizeFromAnySide|
                              ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar);
  ImGui::SetWindowPos(ImVec2((HexEdit_WindowWidth),(h/2)));
  ImGui::SetWindowSize(ImVec2(HexView_WindowWidth, h/2));

  DrawHexTable();
  ImGui::End();

  ImGui::Begin("debug info: ", NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_ResizeFromAnySide|
                                     ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar);
  ImGui::SetWindowPos(ImVec2((HexEdit_WindowWidth + HexView_WindowWidth), h/2));
  ImGui::SetWindowSize(ImVec2(HexGraph_WindowWidth, h/2));

  auto str = std::to_string(m_delta) + " ms";
  ImGui::Text("time per frame: %s", str.c_str());

  ImGui::InputFloat("text scale", &ImGui::GetFont()->Scale, 0.1f, 0.1f, 3, ImGuiInputTextFlags_EnterReturnsTrue);

  static bool demo_wnd = false;
  ImGui::Checkbox("demo window", &demo_wnd);
  if(demo_wnd) {
    ImGui::ShowDemoWindow();
  }
  ImGui::ShowFontSelector("Font Selector");
  ImGui::ShowStyleSelector("Style Selector");

  ImGui::InputInt("test a", &a);
  ImGui::InputInt("test a end", &a_e);

  ImGui::Text("rows: %d", Columns);
  ImGui::Text("addr: %d", a);
  ImGui::Text("row/col: %lu, %lu", getRow(a), getCol(a));
  ImGui::Text("top: %f, %f", getTopX(a), getTopY(a));
  ImGui::Text("bottom: %f, %f", getBottomX(a), getBottomY(a));

  ImGui::Text("GlyphWidth: %f", GlyphWidth);
  ImGui::Text("Spacing: %f", SpacingBetweenMidColumns);

  float foo,bar,baz;
  SDL_GetDisplayDPI(0,&foo,&bar,&baz);
  ImGui::Text("dpi: %f %f %f", foo, bar, baz);

  ImGui::End();
}

#ifdef _MSC_VER
#define _PRISizeT   "IX"
#else
#define _PRISizeT   "zX"
#endif

void HexEdit::DrawRightClickPopup() {
  if(ImGui::IsMouseClicked(1)) {
    ImGui::OpenPopup("##contextmenu");
  }
  if (ImGui::BeginPopup("##contextmenu"))
  {
    ImGui::PushItemWidth(60);
    if(ImGui::Button("create view")) {
      HexView hv;
      hv.id = m_views.size();
      auto name = "New View " + std::to_string(hv.id);

      if(name.size() > sizeof(hv.name)-1)
        name.resize(sizeof(hv.name)-1);

      strcpy(hv.name, name.data());
      hv.start = std::min(m_click_start, m_click_current);
      hv.end = std::max(m_click_start, m_click_current);
      hv.color = ImColor(IM_COL32(0,128,128,255));
      m_views.push_back(hv);

      m_clicked = false;
      m_click_start = 0;
      m_click_current = 0;

      m_selected_view = m_views.size()-1;

      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void HexEdit::DrawHexEdit() {
  CalcSizes();
  ImGuiStyle& style = ImGui::GetStyle();

  DrawRightClickPopup();

  const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
  ImGui::BeginChild("##scrolling", ImVec2(0, -footer_height_to_reserve));
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

  const int line_total_count = (int)((mem_size + Columns - 1) / Columns);
  ImGuiListClipper clipper(line_total_count, LineHeight);

  const size_t visible_start_addr = clipper.DisplayStart * Columns;
  const size_t visible_end_addr = clipper.DisplayEnd * Columns;

  // Draw vertical separator
  ImVec2 window_pos = ImGui::GetWindowPos();
  if (OptShowAscii)
    draw_list->AddLine(ImVec2(window_pos.x + PosAsciiStart - GlyphWidth, window_pos.y),
                       ImVec2(window_pos.x + PosAsciiStart - GlyphWidth, window_pos.y + 9999), ImGui::GetColorU32(ImGuiCol_Border));

  const ImU32 color_text = ImGui::GetColorU32(ImGuiCol_Text);
  const ImU32 color_disabled = OptGreyOutZeroes ? ImGui::GetColorU32(ImGuiCol_TextDisabled) : color_text;

  // highlights a byte, if there's a view for it
  // returns whether the byte was highlighted or not
  auto highlight_fnc = [&](HexView& v) {
    auto min = std::max((size_t)0, v.start);
    auto max = std::min(v.end, mem_size);

    switch(v.mode) {
      case HexViewMode_Filled: {
        if(getRow(max) - getRow(min) == 0) {
          draw_list->AddRectFilled(ImVec2(window_pos.x + PosHexStart + getTopX(min), window_pos.y + getTopY(min)),
                                   ImVec2(window_pos.x + PosHexStart + getBottomX(max), window_pos.y + getBottomY(max)),
                                   ImColor(v.color));
        } else {
          draw_list->AddRectFilled(ImVec2(window_pos.x + PosHexStart + getTopX(min), window_pos.y + getTopY(min)),
                                   ImVec2(window_pos.x + PosHexStart + getBottomX(Columns - 1), window_pos.y + getBottomY(min)),
                                   ImColor(v.color));

          if(getRow(max) - getRow(min) > 1) {
            draw_list->AddRectFilled(ImVec2(window_pos.x + PosHexStart, window_pos.y + (getRow(min)+1)*LineHeight),
                                     ImVec2(window_pos.x + PosHexStart + getBottomX(Columns - 1), window_pos.y + (getRow(max))*LineHeight),
                                     ImColor(v.color));
          }

          draw_list->AddRectFilled(ImVec2(window_pos.x + PosHexStart, window_pos.y + getTopY(max)),
                                   ImVec2(window_pos.x + PosHexStart + getBottomX(max), window_pos.y + getBottomY(max)),
                                   ImColor(v.color));
        }
        break;
      }
      case HexViewMode_Line: {
        if(getRow(max) - getRow(min) == 0) {
          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getTopX(min), window_pos.y + getTopY(min)),
                             ImVec2(window_pos.x + PosHexStart + getBottomX(max), window_pos.y + getTopY(max)),
                             ImColor(v.color), 3.0f);
          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getTopX(min), window_pos.y + getBottomY(min)),
                             ImVec2(window_pos.x + PosHexStart + getBottomX(max), window_pos.y + getBottomY(max)),
                             ImColor(v.color), 3.0f);
          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getTopX(min), window_pos.y + getTopY(min)),
                             ImVec2(window_pos.x + PosHexStart + getTopX(min), window_pos.y + getBottomY(min)),
                             ImColor(v.color), 3.0f);
          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getBottomX(max), window_pos.y + getTopY(max)),
                             ImVec2(window_pos.x + PosHexStart + getBottomX(max), window_pos.y + getBottomY(max)),
                             ImColor(v.color), 3.0f);
        } else {
          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getTopX(min), window_pos.y + getTopY(min)),
                             ImVec2(window_pos.x + PosHexStart + getBottomX(Columns-1), window_pos.y + getTopY(min)),
                             ImColor(v.color), 3.0f);
          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getTopX(min), window_pos.y + getBottomY(min)),
                             ImVec2(window_pos.x + PosHexStart, window_pos.y + getBottomY(min)),
                             ImColor(v.color), 3.0f);
          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getTopX(min), window_pos.y + getTopY(min)),
                             ImVec2(window_pos.x + PosHexStart + getTopX(min), window_pos.y + getBottomY(min)),
                             ImColor(v.color), 3.0f);
          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getBottomX(Columns-1), window_pos.y + getTopY(min)),
                             ImVec2(window_pos.x + PosHexStart + getBottomX(Columns-1), window_pos.y + getBottomY(min)),
                             ImColor(v.color), 3.0f);

          if(getRow(max) - getRow(min) > 1) {
            draw_list->AddLine(ImVec2(window_pos.x + PosHexStart, window_pos.y + getBottomY(min)),
                               ImVec2(window_pos.x + PosHexStart, window_pos.y + getTopY(max)),
                               ImColor(v.color), 3.0f);
            draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getBottomX(Columns-1), window_pos.y + getBottomY(min)),
                               ImVec2(window_pos.x + PosHexStart + getBottomX(Columns-1), window_pos.y + getTopY(max)),
                               ImColor(v.color), 3.0f);
          }

          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getBottomX(max), window_pos.y + getBottomY(max)),
                             ImVec2(window_pos.x + PosHexStart, window_pos.y + getBottomY(max)),
                             ImColor(v.color), 3.0f);
          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getBottomX(max), window_pos.y + getTopY(max)),
                             ImVec2(window_pos.x + PosHexStart + getBottomX(Columns-1), window_pos.y + getTopY(max)),
                             ImColor(v.color), 3.0f);
          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart + getBottomX(max), window_pos.y + getBottomY(max)),
                             ImVec2(window_pos.x + PosHexStart + getBottomX(max), window_pos.y + getTopY(max)),
                             ImColor(v.color), 3.0f);
          draw_list->AddLine(ImVec2(window_pos.x + PosHexStart, window_pos.y + getBottomY(max)),
                             ImVec2(window_pos.x + PosHexStart, window_pos.y + getTopY(max)),
                             ImColor(v.color), 3.0f);
        }
        break;
      }
    }

  };

  HexView hv;
  strcpy(hv.name, "New View");
  hv.start = std::min(m_click_start, m_click_current);
  hv.end = std::max(m_click_start, m_click_current);
  hv.color = ImColor(IM_COL32(255,0,0,128));
  hv.mode = HexViewMode_Line;

  // highlight current selection
  highlight_fnc(hv);
  // highlight views
  for(size_t i=0; i < m_views.size(); i++) {
    highlight_fnc(m_views[i]);
  }

  // render all visible lines
  for (int line_i = clipper.DisplayStart; line_i < clipper.DisplayEnd; line_i++)
  {
    // calculate first address of line
    size_t addr = (size_t)(line_i * Columns);
    // render first line number
    ImGui::Text("%0*" _PRISizeT ": ", (int)AddrDigitsCount, base_display_addr + addr);

    // render all hex numbers
    for (int n = 0; n < Columns && addr < mem_size; n++, addr++)
    {
      float uint8_t_pos_x = PosHexStart + HexCellWidth * n;
      if (OptMidColumnsCount > 0)
        uint8_t_pos_x += (n / OptMidColumnsCount) * SpacingBetweenMidColumns;
      ImGui::SameLine(uint8_t_pos_x);

      // read current byte
      uint8_t b = ReadFn(mem_data, addr);

      auto handleTooltipAndClick = [&]() {
        m_current_view = isHighlighted(addr);

        // tooltip
        if (ImGui::IsItemHovered()) {
          if(m_current_view >= 0 && (size_t)m_current_view < m_views.size()) {
            ImGui::SetTooltip("%s | %lu bytes", m_views[m_current_view].name,
                              m_views[m_current_view].end - m_views[m_current_view].start);
          }
        }

        // text selection
        if (ImGui::IsMouseDown(0)) {
          if (ImGui::IsItemHovered()) {
            if(m_current_view >= 0 && (size_t)m_current_view < m_views.size()) {
              m_selected_view = m_views[m_current_view].id;
            }

            if (!m_clicked) {
              m_clicked = true;
              m_click_start = addr;
              m_click_current = addr;
            } else {
              m_click_current = addr;
            }
          }
          ImGui::SetTooltip("%lu bytes", std::max(m_click_start, m_click_current) - std::min(m_click_start, m_click_current));
        } else {
          m_clicked = false;
        }
      };

      if (b == 0 && OptGreyOutZeroes) {
        ImGui::TextDisabled("0");
        handleTooltipAndClick();
        ImGui::SameLine(uint8_t_pos_x + GlyphWidth);
        ImGui::TextDisabled("0 ");
        handleTooltipAndClick();
      }
      else {
        ImGui::Text("%01X", b >> 4);
        handleTooltipAndClick();

        ImGui::SameLine(uint8_t_pos_x + GlyphWidth);

        ImGui::Text("%01X ", b & 0x0F);
        handleTooltipAndClick();
      }

      ImGui::SetKeyboardFocusHere();
      ImGui::CaptureKeyboardFromApp(true);
      if(ImGui::GetKeyPressedAmount(ImGui::GetKeyIndex(ImGuiKey_LeftArrow), 100, 1)) {
      }
      if(ImGui::GetKeyPressedAmount(ImGui::GetKeyIndex(ImGuiKey_RightArrow), 100, 1)) {
      }

    }

    if (OptShowAscii)
    {
      // Draw ASCII values
      ImGui::SameLine(PosAsciiStart);
      ImVec2 pos = ImGui::GetCursorScreenPos();
      addr = line_i * Columns;
      ImGui::PushID(line_i);
      if (ImGui::InvisibleButton("ascii", ImVec2(PosAsciiEnd - PosAsciiStart, LineHeight)))
      {
        //DataEditingAddr = addr + (size_t)((ImGui::GetIO().MousePos.x - pos.x) / s.GlyphWidth);
        //DataEditingTakeFocus = true;
      }
      ImGui::PopID();
      for (int n = 0; n < Columns && addr < mem_size; n++, addr++)
      {
        unsigned char c = ReadFn(mem_data, addr);
        char display_c = (c < 32 || c >= 128) ? '.' : c;
        draw_list->AddText(pos, (display_c == '.') ? color_disabled : color_text, &display_c, &display_c + 1);
        pos.x += GlyphWidth;
      }
    }

  }
  clipper.End();
  ImGui::PopStyleVar(2);
  ImGui::EndChild();

  ImGui::Separator();

  ImGui::Text("Position: 0");

  ImGui::SameLine();
  ImGui::Text("Range %0*" _PRISizeT "..%0*" _PRISizeT, AddrDigitsCount, base_display_addr, AddrDigitsCount, base_display_addr + mem_size - 1);
  ImGui::SameLine();
  ImGui::PushItemWidth((AddrDigitsCount + 1) * GlyphWidth + style.FramePadding.x * 2.0f);
  if (ImGui::InputText("##addr", AddrInputBuf, 32, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue))
  {
    size_t goto_addr;
    if (sscanf(AddrInputBuf, "%" _PRISizeT, &goto_addr) == 1)
    {
      GotoAddr = goto_addr - base_display_addr;
    }
  }
  ImGui::PopItemWidth();

  if (GotoAddr != (size_t)-1)
  {
    if (GotoAddr < mem_size)
    {
      ImGui::BeginChild("##scrolling");
      ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + (GotoAddr / Columns) * ImGui::GetTextLineHeight());
      ImGui::EndChild();
    }
    GotoAddr = (size_t)-1;
  }

  // Notify the main window of our ideal child content size (FIXME: we are missing an API to get the contents size from the child)
  ImGui::SetCursorPosX(HexEdit_WindowWidth);
}

void HexEdit::DrawHexView() {
  ImGui::BeginChild("viewlist", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, m_height * 0.4f), false);
  for (size_t n = 0; n < m_views.size(); n++)
  {
    char buf[sizeof(m_views[n].name) + 32];
    snprintf(buf, sizeof(buf), "%0*" _PRISizeT " : %s", (int)AddrDigitsCount, m_views[n].start, m_views[n].name);
    if (ImGui::Selectable(buf, n==m_selected_view))
      m_selected_view = n;

    if (n==m_selected_view)
      ImGui::SetItemDefaultFocus();
  }
  ImGui::EndChild();

  if(m_views.size()) {
    if(m_selected_view >= m_views.size())
      m_selected_view = 0;

    ImGui::SameLine();

    ImGui::BeginChild("vieweditor", ImVec2(0,m_height * 0.4f), false);
    ImGui::InputText("name", m_views[m_selected_view].name, sizeof(m_views[m_selected_view].name));
    ImGui::ColorEdit4("color", &m_views[m_selected_view].color.x);
    ImGui::InputInt("start", (int*)&m_views[m_selected_view].start, 1, 16,
                    ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::InputInt("end", (int*)&m_views[m_selected_view].end, 1, 16,
                    ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue);

    const char* items[] = { "Filled", "Line" };
    int item_current;
    switch(m_views[m_selected_view].mode) {
      case HexViewMode_Filled: {
        item_current = 0;
        break;
      }
      case HexViewMode_Line: {
        item_current = 1;
        break;
      }
    }
    ImGui::Combo("combo", &item_current, items, IM_ARRAYSIZE(items));
    switch(item_current) {
      case 0: {
        m_views[m_selected_view].mode = HexViewMode_Filled;
        break;
      }
      case 1: {
        m_views[m_selected_view].mode = HexViewMode_Line;
        break;
      }
    }

    ImGui::Text("size: %lu", m_views[m_selected_view].end - (m_views[m_selected_view].start - 1));
    //todo: value
    //ImGui::InputText("hexadecimal", 0,0, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);

    ImGui::EndChild();
  }
}

void HexEdit::DrawHexGraph() {

  ImGui::Image(0, ImVec2(m_width/3, m_height/2));
}

void HexEdit::DrawHexTable() {
  {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    //ImGui::BeginChild("HexTable", ImVec2(0,300), true);
    ImGui::Columns(data_X.size() + 1);

    ImGui::Text(" ");
    ImGui::NextColumn();

    char buf[32];

    for (size_t x=0; x < data_X.size(); x++)
    {
      snprintf(buf, sizeof(buf), "##x%lu", x);
      ImGui::InputFloat(buf, &data_X[x], 0.0f, 0.0f, 2, ImGuiInputTextFlags_EnterReturnsTrue);
      ImGui::NextColumn();
    }

    ImGui::Separator();

    for(size_t y=0; y < data_Y.size(); y++) {
      snprintf(buf, sizeof(buf), "##y%lu", y);
      ImGui::InputFloat(buf, &data_Y[y], 0.0f, 0.0f, 2, ImGuiInputTextFlags_EnterReturnsTrue);
      ImGui::NextColumn();

      for (size_t x = 0; x < data_X.size(); x++)
      {
        snprintf(buf, sizeof(buf), "##d%lu", x+(y*data_X.size()));
        ImGui::InputFloat(buf, &data[x + y * data_X.size()], 0.0f, 0.0f, 2, ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::NextColumn();
      }
    }

    //ImGui::EndChild();
    ImGui::PopStyleVar();
  }
}

#undef _PRISizeT
