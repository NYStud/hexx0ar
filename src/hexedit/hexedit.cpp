#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <cstring>
#include <application/log.hpp>
#include "hexedit.hpp"

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
  LineHeight = ImGui::GetTextLineHeight();
  GlyphWidth = ImGui::CalcTextSize("F").x + 1;                  // We assume the font is mono-space
  HexCellWidth = (float)(int)(GlyphWidth * 2.5f);             // "FF " we include trailing space in the width to easily catch clicks everywhere
  SpacingBetweenMidRows = (float)(int)(HexCellWidth * 0.25f); // Every OptMidRowsCount columns we add a bit of extra spacing
  PosHexStart = (AddrDigitsCount + 2) * GlyphWidth;
  PosHexEnd = PosHexStart + (HexCellWidth * Rows);
  PosAsciiStart = PosAsciiEnd = PosHexEnd;
  if (OptShowAscii)
  {
    PosAsciiStart = PosHexEnd + GlyphWidth * 1;
    if (OptMidRowsCount > 0)
      PosAsciiStart += ((Rows + OptMidRowsCount - 1) / OptMidRowsCount) * SpacingBetweenMidRows;
    PosAsciiEnd = PosAsciiStart + Rows * GlyphWidth;
  }
  WindowWidth = PosAsciiEnd + style.ScrollbarSize + style.WindowPadding.x * 2 + GlyphWidth;
}

void HexEdit::BeginWindow(const char *title, size_t w, size_t h) {
  m_width = w;
  m_height = h;

  CalcSizes();
  ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(WindowWidth, FLT_MAX));

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
    DrawHexEditContents();
    if (ContentsWidthChanged)
    {
      CalcSizes();
      ImGui::SetWindowSize(ImVec2(WindowWidth, ImGui::GetWindowSize().y));
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
  DrawHexGraphContents();
  ImGui::End();
}

#ifdef _MSC_VER
#define _PRISizeT   "IX"
#else
#define _PRISizeT   "zX"
#endif

void HexEdit::DrawHexEditContents() {
  CalcSizes();
  ImGuiStyle& style = ImGui::GetStyle();

  const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
  ImGui::BeginChild("##scrolling", ImVec2(0, -footer_height_to_reserve));
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

  const int line_total_count = (int)((mem_size + Rows - 1) / Rows);
  ImGuiListClipper clipper(line_total_count, LineHeight);

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

  // Draw vertical separator
  ImVec2 window_pos = ImGui::GetWindowPos();
  if (OptShowAscii)
    draw_list->AddLine(ImVec2(window_pos.x + PosAsciiStart - GlyphWidth, window_pos.y), ImVec2(window_pos.x + PosAsciiStart - GlyphWidth, window_pos.y + 9999), ImGui::GetColorU32(ImGuiCol_Border));

  //const ImU32 color_text = ImGui::GetColorU32(ImGuiCol_Text);
  //const ImU32 color_disabled = OptGreyOutZeroes ? ImGui::GetColorU32(ImGuiCol_TextDisabled) : color_text;

  for (int line_i = clipper.DisplayStart; line_i < clipper.DisplayEnd; line_i++) // display only visible lines
  {
    size_t addr = (size_t)(line_i * Rows);
    ImGui::Text("%0*" _PRISizeT ": ", (int)AddrDigitsCount, base_display_addr + addr);

    // Draw Hexadecimal
    for (int n = 0; n < Rows && addr < mem_size; n++, addr++)
    {
      float uint8_t_pos_x = PosHexStart + HexCellWidth * n;
      if (OptMidRowsCount > 0)
        uint8_t_pos_x += (n / OptMidRowsCount) * SpacingBetweenMidRows;
      ImGui::SameLine(uint8_t_pos_x);

      // highlight all views
      auto highlight_fnc = [&](HexView& v) -> bool{
        auto min = v.start;
        auto max = v.end + 1;
        if((addr >= min && addr < max)) {
          ImVec2 pos = ImGui::GetCursorScreenPos();
          float highlight_width = GlyphWidth * 2;
          bool is_next_uint8_t_highlighted =  (addr + 1 < mem_size) && ((max != (size_t)-1 && addr + 1 < max));
          if (is_next_uint8_t_highlighted || (n + 1 == Rows))
          {
            highlight_width = HexCellWidth;
            if (OptMidRowsCount > 0 && n > 0 && (n + 1) < Rows && ((n + 1) % OptMidRowsCount) == 0)
              highlight_width += SpacingBetweenMidRows;
          }

          draw_list->AddRectFilled(pos, ImVec2(pos.x + highlight_width, pos.y + LineHeight), ImColor(v.color));

          return true;
        } else {
          return false;
        }
      };

      HexView hv;
      strcpy(hv.name, "New View");
      hv.start = std::min(m_click_start, m_click_current);
      hv.end = std::max(m_click_start, m_click_current);
      hv.color = ImColor(IM_COL32(255,0,0,128));

      int m_current_view = -1;
      highlight_fnc(hv);
      for(int i=0; i < m_views.size(); i++) {
        if(highlight_fnc(m_views[i])) {
          m_current_view = i;
        }
      }

      // NB: The trailing space is not visible but ensure there's no gap that the mouse cannot click on.
      uint8_t b = ReadFn(mem_data, addr);

      auto handleTooltipAndClick = [&]() {
        // tooltip
        if (ImGui::IsItemHovered()) {
          if(m_current_view >= 0 && m_current_view < m_views.size()) {
            ImGui::SetTooltip("%s", m_views[m_current_view].name);
          }
        }

        // text selection
        if (ImGui::IsMouseDown(0)) {
          if (ImGui::IsItemHovered()) {
            if(m_current_view >= 0 && m_current_view < m_views.size()) {
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

    /*
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
    */

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

    //ImGui::Checkbox("Show HexII", &OptShowHexII);
    //if (ImGui::Checkbox("Show Ascii", &OptShowAscii)) ContentsWidthChanged = true;
    ImGui::Checkbox("Grey out zeroes", &OptGreyOutZeroes);

    ImGui::EndPopup();
  }

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
  ImGui::SetCursorPosX(WindowWidth);
}

void HexEdit::DrawHexViewContents() {
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

    ImGui::BeginChild("vieweditor", ImVec2(0,m_height * 0.4f), true);
    ImGui::InputText("name", m_views[m_selected_view].name, sizeof(m_views[m_selected_view].name));
    ImGui::ColorEdit4("color", &m_views[m_selected_view].color.x);
    ImGui::InputInt("start", (int*)&m_views[m_selected_view].start, 1, 16, ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::InputInt("end", (int*)&m_views[m_selected_view].end, 1, 16, ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::Text("size: %d", m_views[m_selected_view].end - (m_views[m_selected_view].start - 1));
    //todo: value
    //ImGui::InputText("hexadecimal", 0,0, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);

    ImGui::EndChild();
  }
}

void HexEdit::DrawHexGraphContents() {
  ImGui::Image(0, ImVec2(m_width/3, m_height/2));
}

#undef _PRISizeT
