#pragma once

#include <imgui.h>
#include <SDL2/SDL_scancode.h>
#include <helpers/signal.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include "opengl/glclasses.hpp"
#include "opengl/shader.hpp"

/*
 * simple gl3 imgui renderer
 * */
class UIRenderer {
private:
  gl::Shader m_shader;
  gl::Texture m_fonttex;
  gl::VAO m_vao;
  gl::VBO m_vbo;
  gl::IBO m_ibo;

  glm::mat4 m_projection_matrix;

public:
  UIRenderer() {};
  ~UIRenderer() {};
  void init();
  void deinit();
  void update(unsigned int dt);
  void render();

  //x/y are the window size
  //dx/dy are the drawable size (differs maybe on retina displays)
  Signal<void(unsigned int w, unsigned int h, unsigned int dw, unsigned int dh)> onResize;
  Signal<void(bool up, bool pressed, unsigned int scancode)> onKey;
  Signal<void(int x, int y, int xrel, int yrel)> onMove;
  Signal<void(bool up, int x, int y, uint8_t button, uint8_t state, uint8_t clicks)> onClick;
  Signal<void(int x, int y)> onWheel;
  Signal<void(std::string text)> onTextInput;
};
