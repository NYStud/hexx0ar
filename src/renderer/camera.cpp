#include <glm/gtc/matrix_transform.hpp>
#include "camera.hpp"

void Camera::init() {
  m_view_matrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -1.0f));
}

void Camera::move(float x, float y) {
  m_view_matrix = glm::translate(m_view_matrix, glm::vec3(x, y, 0.0f));
}

void Camera::rotate(float angle, glm::vec3 axis) {
  m_view_matrix = glm::rotate(m_view_matrix, angle, axis);
}

void Camera::scale(float factor) {

  m_view_matrix = glm::scale(m_view_matrix, glm::vec3(factor,factor,1.0f));
}

void Camera::lookAt(float x, float y) {
  m_view_matrix = glm::translate(glm::mat4(), glm::vec3(x, y, -1.0f));
}

glm::mat4 Camera::getMatrix() {
  return m_view_matrix;
}
