#pragma once

#include <renderer/uirenderer.hpp>
#include <sdlwrapper/sdlwindow.hpp>

/*
 * the application handles the initial main thread and dispatches the three threads for network, script/physic and
 * rendering.
 *
 * */
class Application {
private:
  bool m_running = false;

  unsigned int m_ticks = 0;
  unsigned int m_delta = 1;

  sdl2::SDLWindow m_wnd;

  UIRenderer m_uirenderer;

public:
  Application() {}
  void start(int argc, const char** argv);
  void stop();
};
