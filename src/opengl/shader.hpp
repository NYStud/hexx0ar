#pragma once

#include "gltypelookup.hpp"
#include "glclasses.hpp"
#include "shaderpreprocessor.hpp"

#include <epoxy/gl.h>
#include <fstream>
#include <glm/detail/type_mat.hpp>
#include <glm/vec2.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace gl {

class Uniform {
private:
  const GLint m_location;
  const GLsizei m_count;
  const GLenum m_type;
  gl::uniform_method m_uniform_method;

protected:
public:
  Uniform(const GLint m_location, const GLsizei count, const GLenum type);

  template <typename T> void set(T data) const {
#ifdef DEBUG
    if (!valid_uniform_type<std::decay<T>::type>(m_type)) {
      throw std::runtime_error("calling uniform method with wrong type!");
    }
#endif
    auto method = std::get<gl::uniform_type_lookup<typename std::decay<T>::type>::type>(m_uniform_method);
    method(m_location, m_count, data);
  }

  template <typename T> void setMatrix(T data, GLboolean transpose) const {
#ifdef DEBUG
    if (!valid_uniform_type<std::decay<T>::type>(m_type)) {
      throw std::runtime_error("calling uniform method with wrong type!");
    }
#endif
    auto method = std::get<gl::uniform_type_lookup<typename std::decay<T>::type>::type>(m_uniform_method);
    method(m_location, m_count, transpose, data);
  }

  // TODO:
  // template<typename T>
  // T get();

  GLint location() { return m_location; }
  GLsizei count() { return m_count; }
  GLenum type() { return m_type; }
};

/*
 *
 * shader wrapper with the ability of adding defines before compiling using the shader preprocessor.
 *
 * */
class Shader {
private:
  GLuint m_program;
  ShaderPreprocessor m_preprocessor;

  // checks the link status and prints an error, if sth. went wrong
  bool checkLinkStatus();

  // checks the compile status for the shader and prints an error, if sth. went wrong
  bool checkShaderCompileStatus(unsigned int shader);

protected:
public:
  Shader();

  ~Shader();

  // creates the program object
  void init();

  void deinit();

  // binds the shader
  void bind();

  // unbinds the shader
  void unbind();

  // just loads the shader data to a spp lib
  void addData(const std::string& name, const std::string& data);

  // just loads the shader data to a spp lib
  void addFile(const std::string& name, const std::string& path);

  // compiles the shader
  bool compile(const std::string path, GLenum shadertype);

  // link the shaders
  bool link();

  // glUniformLocation wrapper
  int location(const std::string& name);
  int attrib_location(const std::string& name);

  // get the current preprocessor output of the shaderfile (for debugging/testing)
  std::string preprocessorOutput(const std::string& name);

  // returns the program id
  unsigned int program();

  // automatically filled on linking the shader
  std::map<std::string, Uniform> uniforms;

  // the attributes need to be set before linking the shader
  std::map<GLuint, std::string> attributes;
};
}
