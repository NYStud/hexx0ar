#include "application.hpp"
#include "imgui_memory_editor.h"

void khr_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg,
                        const void *data) {
  (void) id;
  (void) length;
  (void) data;
  LOG_SHORT_DEBUG("src " + std::to_string(source) + " " +
                  "type " + std::to_string(type) + " " +
                  "severity " + std::to_string(severity) + " " +
                  std::string(msg))
}

void Application::start(int argc, const char** argv) {

  m_wnd.onClose.connect([this](){ m_running = false; });
  m_wnd.onResize.connect([this](unsigned int w, unsigned int h, unsigned int dw, unsigned int dh) {
    m_uirenderer.onResize(w, h, dw, dh);
  });
  m_wnd.onClick.connect([this](bool up, int x, int y, uint8_t button, uint8_t state, uint8_t clicks) {
    m_uirenderer.onClick(up, x, y, button, state, clicks);
  });
  m_wnd.onMove.connect([this](int x, int y, int xrel, int yrel) {
    m_uirenderer.onMove(x, y, xrel, yrel);
  });
  m_wnd.onWheel.connect([this](int x, int y) {
    m_uirenderer.onWheel(x, y);
  });
  m_wnd.onKey.connect(
    [this](bool up, bool pressed, unsigned int scancode) {
      m_uirenderer.onKey(up, pressed, scancode);
    });
  m_wnd.onTextInput.connect([this](std::string text) { m_uirenderer.onTextInput(text); });

  m_wnd.open();
  m_wnd.setTitle("bluebuild");

  glDebugMessageCallback(&khr_debug_callback, NULL);

  m_uirenderer.init();

  m_running = true;

  while(m_running) {
    m_ticks = SDL_GetTicks();

    m_wnd.poll();

    glClearColor(0.2f, 0.4f, 0.6f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    m_uirenderer.update(m_delta);

    ImGui::Begin("mainloop: ");
    auto str = std::to_string(m_delta) + " ms";
    ImGui::Text("time per frame: %s", str.c_str());
    ImGui::End();

    static MemoryEditor mem_edit;
    mem_edit.HighlightColor = ImGui::GetColorU32(ImVec4(1,0,0,1));
    mem_edit.HighlightMin = 10;
    mem_edit.HighlightMax = 20;

    ImGui::Begin("Hexedit", NULL, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
      if (ImGui::BeginMenu("View"))
      {
        if (ImGui::MenuItem("New")) { /* Do stuff */ }
        if (ImGui::MenuItem("Close")) { /* Do stuff */ }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    mem_edit.DrawContents((unsigned char*)this, sizeof(*this), (size_t)this);

    ImGui::End();

    //always render the ui last
    m_uirenderer.render();

    m_delta = SDL_GetTicks() - m_ticks;

    m_wnd.swap();
  }

  m_uirenderer.deinit();
}

void Application::stop() {
  m_running = false;
}
