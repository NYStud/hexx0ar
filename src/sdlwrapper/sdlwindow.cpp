#include "sdlwindow.hpp"
#include "application/log.hpp"

namespace sdl2 {
SDLWindow::SDLWindow()
  : m_width(640), m_height(400) {
  LOG("initializing sdl...")

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    LOG("Unable to initialize SDL: " + SDL_GetError())
    throw std::runtime_error(SDL_GetError());
  }
}

void SDLWindow::open() {
  LOG("opening window")
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  m_window = WindowPtr(
    SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height,
                     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_ALLOW_HIGHDPI));
  if (m_window == NULL) {
    LOG("failed initializing the window.");
    throw std::runtime_error(SDL_GetError());
  }

#ifdef DEBUG
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
  m_context = SDL_GL_CreateContext(m_window.get());
  if (m_context == NULL) {
    LOG("failed initializing the context.")
    throw std::runtime_error(SDL_GetError());
  }

  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    LOG("failed initializing sdl image.")
    throw std::runtime_error(SDL_GetError());
  }

  int m_drawable_width = 0;
  int m_drawable_height = 0;
  SDL_GL_GetDrawableSize(m_window.get(), &m_drawable_width, &m_drawable_height);
  onResize(m_width, m_height, (uint32_t) m_drawable_width, (uint32_t) m_drawable_height);

  m_initializedgl = true;
}

SDLWindow::~SDLWindow() {
  LOG("closing window")

  if (m_context != NULL)
    SDL_GL_DeleteContext(m_context);

  IMG_Quit();
  SDL_Quit();
}

void SDLWindow::poll() {
  SDL_Event event;
  while (SDL_PollEvent(&event) != 0)
    handleEvent(event);
}

void SDLWindow::swap() { SDL_GL_SwapWindow(m_window.get()); }

void SDLWindow::handleEvent(SDL_Event event) {
  // TODO
  // handle more events
  switch (event.type) {
    case SDL_WINDOWEVENT: {
      switch (event.window.event) {
        case SDL_WINDOWEVENT_CLOSE: {
          LOG("closing window");
          onClose();
          break;
        }
        case SDL_WINDOWEVENT_RESIZED: {
          LOG("resized window");
          if (m_initializedgl) {
            m_width = (uint32_t) event.window.data1;
            m_height = (uint32_t) event.window.data2;
            int m_drawable_width = 0;
            int m_drawable_height = 0;
            SDL_GL_GetDrawableSize(m_window.get(), &m_drawable_width, &m_drawable_height);
            onResize(m_width, m_height, (uint32_t) m_drawable_width, (uint32_t) m_drawable_height);
            LOG("wnd size: " + std::to_string(m_width) + " " + std::to_string(m_height))
            LOG("draw size: " + std::to_string(m_drawable_width) + " " + std::to_string(m_drawable_height))
          }
          break;
        }
      }
      break;
    }
    case SDL_MOUSEBUTTONDOWN: {
      onClick(false, event.button.x, event.button.y, event.button.button, event.button.state, event.button.clicks);
      break;
    }
    case SDL_MOUSEBUTTONUP: {
      onClick(true, event.button.x, event.button.y, event.button.button, event.button.state, event.button.clicks);
      break;
    }
    case SDL_KEYDOWN: {
      onKey(false, event.key.state == SDL_PRESSED, event.key.keysym.scancode);
      break;
    }
    case SDL_KEYUP: {
      onKey(true, event.key.state == SDL_PRESSED, event.key.keysym.scancode);
      break;
    }
    case SDL_MOUSEWHEEL: {
      if (event.wheel.direction == SDL_MOUSEWHEEL_NORMAL)
        onWheel(event.wheel.x, event.wheel.y);
      else
        onWheel(-event.wheel.x, -event.wheel.y);

      break;
    }
    case SDL_MOUSEMOTION: {
      onMove(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
      break;
    }
    case SDL_TEXTINPUT: {
      onTextInput(event.text.text);
      break;
    }
  }
}

unsigned int SDLWindow::getWidth() { return m_width; }

unsigned int SDLWindow::getHeight() { return m_height; }

void SDLWindow::setWidth(uint32_t w) {
  m_width = w;
  SDL_SetWindowSize(m_window.get(), w, m_height);
}

void SDLWindow::setHeight(uint32_t h) {
  m_height = h;
  SDL_SetWindowSize(m_window.get(), m_width, h);
}

bool SDLWindow::initializedGL() { return m_initializedgl; }

void SDLWindow::setTitle(std::string title) { SDL_SetWindowTitle(m_window.get(), title.c_str()); }

std::string SDLWindow::getTitle() {
  const char *c = SDL_GetWindowTitle(m_window.get());
  std::string b;
  if (c != NULL)
    b = c;

  return b;
}

uint8_t SDLWindow::getMouseButton() {
  // todo
  uint8_t b = 0;

  return b;
}

uint8_t SDLWindow::getMouseState() {
  // todo
  uint8_t b = 0;

  return b;
}

}
