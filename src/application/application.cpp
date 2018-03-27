#include "application.hpp"
#include "hexedit/hexedit.hpp"

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
  m_wnd.setTitle("hexx0ar");

  glDebugMessageCallback(&khr_debug_callback, NULL);

  m_uirenderer.init();

  m_running = true;

  static HexEdit hexedit;
  hexedit.Highlights.emplace_back(std::make_tuple(30, 50, IM_COL32(255,0,0,40)));
  //hexedit.Highlights.emplace_back(std::make_tuple(100, 180, IM_COL32(0,255,0,40)));

  while(m_running) {
    m_ticks = SDL_GetTicks();

    m_wnd.poll();

    glClearColor(0.2f, 0.4f, 0.6f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    m_uirenderer.update(m_delta);

    ImGui::Begin("debug info: ");
    auto str = std::to_string(m_delta) + " ms";
    ImGui::Text("time per frame: %s", str.c_str());
    ImGui::Text("clicked %d", hexedit.Clicked);
    ImGui::Text("clickstart %d", hexedit.ClickStartPos);
    ImGui::Text("clickpos %d", hexedit.ClickCurrentPos);
    ImGui::End();

    ImGui::Begin("Hexedit", NULL, ImGuiWindowFlags_MenuBar|ImGuiWindowFlags_NoMove);

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

    hexedit.ReadOnly = true;
    hexedit.DrawContents((unsigned char*)this, sizeof(*this), (size_t)this);

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
