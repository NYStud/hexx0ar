#pragma once

//TODO: evaluate better methods
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtx/transform.hpp>

/*
 * a very simple 2d camera class.
 *
 * generates fitting mvp matrix.
 * */
class Camera {
private:
  glm::mat4 m_view_matrix;
public:

  void init();

  void move(float x, float y);

  void rotate(float angle, glm::vec3 axis);

  void scale(float factor);

  void lookAt(float x, float y);

  glm::mat4 getMatrix();
};

