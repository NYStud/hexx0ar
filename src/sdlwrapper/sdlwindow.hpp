#pragma once

#include <functional>
#include <memory>

#include <helpers/signal.hpp>
#include "sdlwrapper.hpp"

namespace sdl2 {

/*
  The window is just a wrapper for managing the SDL window and it's associated OpenGL context.

  important note: it initializes all SDL subsystems (also things like SDL_image/SDL_ttf/etc).
*/
class SDLWindow
{
private:
protected:
  bool m_initializedgl = false;

  uint32_t m_width;
  uint32_t m_height;

  void handleEvent(SDL_Event event);

  sdl2::WindowPtr m_window;
  SDL_GLContext m_context;

public:
  SDLWindow();

  ~SDLWindow();

  // opening the window and initializing gl
  void open();

  /* eventloop */
  void poll();

  // buffer swapping
  void swap();

  /* some getters */
  uint32_t getWidth();

  uint32_t getHeight();

  bool initializedGL();

  void setWidth(uint32_t w);

  void setHeight(uint32_t h);

  void setTitle(std::string title);

  std::string getTitle();

  uint8_t getMouseButton();

  uint8_t getMouseState();

  Signal<void()> onClose;
  //x/y -> sdl_getwindowsize
  //dx/dy -> sdl_getdrawablesize
  Signal<void(unsigned int w, unsigned int h, unsigned int dw, unsigned int dh)> onResize;
  //this doesn't handle multiple mouses/keyboards (all key/mouse events are handled though)
  Signal<void(bool up, bool pressed, unsigned int scancode)> onKey;
  Signal<void(int x, int y, int xrel, int yrel)> onMove;
  Signal<void(bool up, int x, int y, uint8_t button, uint8_t state, uint8_t clicks)> onClick;
  Signal<void(int x, int y)> onWheel;
  Signal<void(std::string text)> onTextInput;
};
}
