#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <cstring>
#include <application/log.hpp>
#include "hexedit.hpp"

namespace fs = boost::filesystem;

HexEdit::HexEdit() {
  // Settings
  Open = true;
  ReadOnly = false;
  Rows = 16;
  OptShowAscii = true;
  OptShowHexII = false;
  OptGreyOutZeroes = true;
  OptMidRowsCount = 8;
  OptAddrDigitsCount = 0;
  ReadFn = [](uint8_t* data, size_t off) -> uint8_t { return (data != 0) ? data[off] : 0; };
  // todo: writefn

  // State/Internals
  ContentsWidthChanged = false;
  memset(DataInputBuf, 0, sizeof(DataInputBuf));
  memset(AddrInputBuf, 0, sizeof(AddrInputBuf));
  GotoAddr = (size_t)-1;
}

void HexEdit::CalcSizes(Sizes &s) {
  ImGuiStyle& style = ImGui::GetStyle();
  s.AddrDigitsCount = OptAddrDigitsCount;
  if (s.AddrDigitsCount == 0)
    for (size_t n = base_display_addr + mem_size - 1; n > 0; n >>= 4)
      s.AddrDigitsCount++;
  s.LineHeight = ImGui::GetTextLineHeight();
  s.GlyphWidth = ImGui::CalcTextSize("F").x + 1;                  // We assume the font is mono-space
  s.HexCellWidth = (float)(int)(s.GlyphWidth * 2.5f);             // "FF " we include trailing space in the width to easily catch clicks everywhere
  s.SpacingBetweenMidRows = (float)(int)(s.HexCellWidth * 0.25f); // Every OptMidRowsCount columns we add a bit of extra spacing
  s.PosHexStart = (s.AddrDigitsCount + 2) * s.GlyphWidth;
  s.PosHexEnd = s.PosHexStart + (s.HexCellWidth * Rows);
  s.PosAsciiStart = s.PosAsciiEnd = s.PosHexEnd;
  if (OptShowAscii)
  {
    s.PosAsciiStart = s.PosHexEnd + s.GlyphWidth * 1;
    if (OptMidRowsCount > 0)
      s.PosAsciiStart += ((Rows + OptMidRowsCount - 1) / OptMidRowsCount) * s.SpacingBetweenMidRows;
    s.PosAsciiEnd = s.PosAsciiStart + Rows * s.GlyphWidth;
  }
  s.WindowWidth = s.PosAsciiEnd + style.ScrollbarSize + style.WindowPadding.x * 2 + s.GlyphWidth;
}

void HexEdit::BeginWindow(const char *title, size_t w, size_t h) {
  Sizes s;
  CalcSizes(s);
  ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(s.WindowWidth, FLT_MAX));

  Open = true;
  if (ImGui::Begin(title, &Open,ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_ResizeFromAnySide|
                                ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar))
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

        if (ImGui::MenuItem("Close")) {

        }
        if (ImGui::MenuItem("Quit")) {
          //yolo
          exit(0);
        }
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

        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Button("Cancel", ImVec2(120,0))) { ImGui::CloseCurrentPopup(); }

      ImGui::EndPopup();
    }

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsMouseClicked(1))
      ImGui::OpenPopup("context");
    DrawHexEditContents();
    if (ContentsWidthChanged)
    {
      CalcSizes(s);
      ImGui::SetWindowSize(ImVec2(s.WindowWidth, ImGui::GetWindowSize().y));
    }
  }
  ImGui::SetWindowPos(ImVec2(0,0));
  if(w && h) {
    ImGui::SetWindowSize(ImVec2(w/3, h));
  }

  ImGui::End();

  ImGui::Begin("View", NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_ResizeFromAnySide|
                             ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar);

  ImGui::SetWindowPos(ImVec2(w/3,0));
  if(w && h) {
    ImGui::SetWindowSize(ImVec2(w/3, h/2));
  }
  DrawHexViewContents();
  ImGui::End();

  ImGui::Begin("Graph", NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_ResizeFromAnySide|
                              ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoScrollbar);
  ImGui::SetWindowPos(ImVec2(2*(w/3),0));
  if(w && h) {
    ImGui::SetWindowSize(ImVec2(w/3, h/2));
  }
  ImGui::End();
}

#ifdef _MSC_VER
#define _PRISizeT   "IX"
#else
#define _PRISizeT   "zX"
#endif

void HexEdit::DrawHexEditContents() {
  Sizes s;
  CalcSizes(s);
  ImGuiStyle& style = ImGui::GetStyle();

  const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
  ImGui::BeginChild("##scrolling", ImVec2(0, -footer_height_to_reserve));
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

  const int line_total_count = (int)((mem_size + Rows - 1) / Rows);
  ImGuiListClipper clipper(line_total_count, s.LineHeight);

  //const size_t visible_start_addr = clipper.DisplayStart * Rows;
  //const size_t visible_end_addr = clipper.DisplayEnd * Rows;

  if(ImGui::IsMouseClicked(1)) {
    ImGui::OpenPopup("##contextmenu");
  }
  if (ImGui::BeginPopup("##contextmenu"))
  {
    ImGui::PushItemWidth(60);
    if(ImGui::Button("create view")) {
      HexView hv;
      strcpy(hv.name, "New View");
      hv.start = std::min(m_click_start, m_click_current);
      hv.end = std::max(m_click_start, m_click_current);
      hv.color = ImVec4(0,255,255,128);
      m_views.push_back(hv);

      m_clicked = false;
      m_click_start = 0;
      m_click_current = 0;
    }
    ImGui::EndPopup();
  }

  // Draw vertical separator
  ImVec2 window_pos = ImGui::GetWindowPos();
  if (OptShowAscii)
    draw_list->AddLine(ImVec2(window_pos.x + s.PosAsciiStart - s.GlyphWidth, window_pos.y), ImVec2(window_pos.x + s.PosAsciiStart - s.GlyphWidth, window_pos.y + 9999), ImGui::GetColorU32(ImGuiCol_Border));

  const ImU32 color_text = ImGui::GetColorU32(ImGuiCol_Text);
  const ImU32 color_disabled = OptGreyOutZeroes ? ImGui::GetColorU32(ImGuiCol_TextDisabled) : color_text;

  for (int line_i = clipper.DisplayStart; line_i < clipper.DisplayEnd; line_i++) // display only visible lines
  {
    size_t addr = (size_t)(line_i * Rows);
    ImGui::Text("%0*" _PRISizeT ": ", s.AddrDigitsCount, base_display_addr + addr);

    // Draw Hexadecimal
    for (int n = 0; n < Rows && addr < mem_size; n++, addr++)
    {
      float uint8_t_pos_x = s.PosHexStart + s.HexCellWidth * n;
      if (OptMidRowsCount > 0)
        uint8_t_pos_x += (n / OptMidRowsCount) * s.SpacingBetweenMidRows;
      ImGui::SameLine(uint8_t_pos_x);

      // hightlight all views
      auto highlight_fnc = [&](HexView& v) {
        auto min = v.start;
        auto max = v.end;
        if((addr >= min && addr < max)) {
          ImVec2 pos = ImGui::GetCursorScreenPos();
          float highlight_width = s.GlyphWidth * 2;
          bool is_next_uint8_t_highlighted =  (addr + 1 < mem_size) && ((max != (size_t)-1 && addr + 1 < max));
          if (is_next_uint8_t_highlighted || (n + 1 == Rows))
          {
            highlight_width = s.HexCellWidth;
            if (OptMidRowsCount > 0 && n > 0 && (n + 1) < Rows && ((n + 1) % OptMidRowsCount) == 0)
              highlight_width += s.SpacingBetweenMidRows;
          }
          draw_list->AddRectFilled(pos, ImVec2(pos.x + highlight_width, pos.y + s.LineHeight), ImGui::GetColorU32(v.color));
        }
      };

      HexView hv;
      strcpy(hv.name, "New View");
      hv.start = std::min(m_click_start, m_click_current);
      hv.end = std::max(m_click_start, m_click_current);
      hv.color = ImVec4(255,0,0,128);

      highlight_fnc(hv);
      for(auto v : m_views)
        highlight_fnc(v);

      // NB: The trailing space is not visible but ensure there's no gap that the mouse cannot click on.
      uint8_t b = ReadFn(mem_data, addr);

      if (OptShowHexII)
      {
        if ((b >= 32 && b < 128))
          ImGui::Text(".%c ", b);
        else if (b == 0xFF && OptGreyOutZeroes)
          ImGui::TextDisabled("## ");
        else if (b == 0x00)
          ImGui::Text("   ");
        else
          ImGui::Text("%02X ", b);
      }
      else
      {
        if (b == 0 && OptGreyOutZeroes)
          ImGui::TextDisabled("00 ");
        else
          ImGui::Text("%02X ", b);
      }

      // text selection
      if(ImGui::IsMouseDown(0)) {
        if (ImGui::IsItemHovered()) {
          if (!m_clicked) {
            m_clicked = true;
            m_click_start = addr;
            m_click_current = addr+1;
          } else {
            m_click_current = addr+1;
          }
        }
      } else {
        m_clicked = false;
      }

    }

    if (OptShowAscii)
    {
      // Draw ASCII values
      ImGui::SameLine(s.PosAsciiStart);
      ImVec2 pos = ImGui::GetCursorScreenPos();
      addr = line_i * Rows;
      ImGui::PushID(line_i);
      if (ImGui::InvisibleButton("ascii", ImVec2(s.PosAsciiEnd - s.PosAsciiStart, s.LineHeight)))
      {
        //DataEditingAddr = addr + (size_t)((ImGui::GetIO().MousePos.x - pos.x) / s.GlyphWidth);
        //DataEditingTakeFocus = true;
      }
      ImGui::PopID();
      for (int n = 0; n < Rows && addr < mem_size; n++, addr++)
      {
        unsigned char c = ReadFn(mem_data, addr);
        char display_c = (c < 32 || c >= 128) ? '.' : c;
        draw_list->AddText(pos, (display_c == '.') ? color_disabled : color_text, &display_c, &display_c + 1);
        pos.x += s.GlyphWidth;
      }
    }

  }
  clipper.End();
  ImGui::PopStyleVar(2);
  ImGui::EndChild();

  ImGui::Separator();

  // Options menu
  if (ImGui::Button("Options"))
    ImGui::OpenPopup("context");
  if (ImGui::BeginPopup("context"))
  {
    ImGui::PushItemWidth(56);
    if (ImGui::DragInt("##rows", &Rows, 0.2f, 4, 32, "%.0f rows")) ContentsWidthChanged = true;
    ImGui::PopItemWidth();

    ImGui::Checkbox("Show HexII", &OptShowHexII);
    if (ImGui::Checkbox("Show Ascii", &OptShowAscii)) ContentsWidthChanged = true;
    ImGui::Checkbox("Grey out zeroes", &OptGreyOutZeroes);

    ImGui::EndPopup();
  }

  ImGui::SameLine();
  ImGui::Text("Range %0*" _PRISizeT "..%0*" _PRISizeT, s.AddrDigitsCount, base_display_addr, s.AddrDigitsCount, base_display_addr + mem_size - 1);
  ImGui::SameLine();
  ImGui::PushItemWidth((s.AddrDigitsCount + 1) * s.GlyphWidth + style.FramePadding.x * 2.0f);
  if (ImGui::InputText("##addr", AddrInputBuf, 32, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue))
  {
    size_t goto_addr;
    if (sscanf(AddrInputBuf, "%" _PRISizeT, &goto_addr) == 1)
    {
      GotoAddr = goto_addr - base_display_addr;
      //HighlightMin = HighlightMax = (size_t)-1;
    }
  }
  ImGui::PopItemWidth();

  if (GotoAddr != (size_t)-1)
  {
    if (GotoAddr < mem_size)
    {
      ImGui::BeginChild("##scrolling");
      ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y + (GotoAddr / Rows) * ImGui::GetTextLineHeight());
      ImGui::EndChild();
    }
    GotoAddr = (size_t)-1;
  }

  // Notify the main window of our ideal child content size (FIXME: we are missing an API to get the contents size from the child)
  ImGui::SetCursorPosX(s.WindowWidth);
}

void HexEdit::DrawHexViewContents() {
  if(m_views.size()) {
    static int selected = 0;

    if(selected >= m_views.size())
      selected = 0;

    if (ImGui::BeginCombo("##hexview", m_views[selected].name)) // The second parameter is the label previewed before opening the combo.
    {
      for (int n = 0; n < m_views.size(); n++)
      {
        if (ImGui::Selectable(m_views[n].name, n==selected))
          selected = n;

        if (n==selected)
          ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
      }
      ImGui::EndCombo();
    }

    ImGui::ColorEdit4("view color", &m_views[selected].color.x);

    ImGui::InputText("name", m_views[selected].name, sizeof(m_views[selected].name));

    //ImGui::InputText("hexadecimal", 0,0, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
  }
}

#undef _PRISizeT
