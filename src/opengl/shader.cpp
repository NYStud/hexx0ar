#include "../opengl/shader.hpp"
#include "application/log.hpp"

gl::Uniform::Uniform(const GLint location, const GLsizei count, const GLenum type)
    : m_location(location)
    , m_count(count)
    , m_type(type) {
  m_uniform_method = getUniformMethod(m_type);
}

gl::Shader::Shader()
    : m_program(0) {}

gl::Shader::~Shader() {}

void gl::Shader::init() { m_program = glCreateProgram(); }

void gl::Shader::deinit() { glDeleteProgram(m_program); m_program = 0; }

bool gl::Shader::checkLinkStatus() {
  GLint len = 0;
  GLint result = 0;

  glGetProgramiv(m_program, GL_LINK_STATUS, &result);
  glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &len);

  if (result == GL_FALSE) {
    GLchar error[len];
    glGetProgramInfoLog(m_program, len, NULL, error);
    LOG_ERROR("errorlog: " + std::string(error));
  }

  return (bool)result;
}

bool gl::Shader::checkShaderCompileStatus(GLuint shader) {
  GLint len = 0;
  GLint result = 0;

  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

  if (result == GL_FALSE) {
    LOG_WARN("shader compilation failed: ")
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    GLchar buf[len];
    glGetShaderInfoLog(shader, len, NULL, buf);
    LOG_WARN(std::string(buf))
  }

  return result != GL_FALSE;
}

void gl::Shader::bind() {
  if (m_program == 0) {
    LOG_ERROR("error: invalid to bind invalid program (0)!")
    return;
  }

  glUseProgram(m_program);
}

GLuint gl::Shader::program() { return m_program; }

void gl::Shader::unbind() { glUseProgram(0); }

void gl::Shader::addData(const std::string& name, const std::string& data) { m_preprocessor.addData(name, data); }

void gl::Shader::addFile(const std::string& name, const std::string& path) { m_preprocessor.addFile(name, path); }

bool gl::Shader::compile(const std::string name, GLenum shadertype) {

  if (m_program == 0) {
    LOG_WARN("shader program is invalid!")
    return false;
  }

  GLuint shader = glCreateShader(shadertype);

  std::string s = m_preprocessor.preprocess(name);
  const char* cs = s.c_str();

  glShaderSource(shader, 1, &cs, NULL);
  glCompileShader(shader);

  bool b = checkShaderCompileStatus(shader);

  glAttachShader(m_program, shader);
  glDeleteShader(shader);

  if (!b)
    return false;

  LOG("successfully loaded shader " + name + "!")

  return true;
}

bool gl::Shader::link() {
  // set the attributes
  // it's not necessary checking them, because non-existent names are ignored
  // however this could be worth a nice little debug log by checking them with glGetActiveAttrib()
  for (auto const &attr : attributes) {
    LOG_DEBUG("attribute at " + std::to_string(attr.first) + " called " + attr.second)
    glBindAttribLocation(m_program, attr.first, attr.second.c_str());
  }

  glLinkProgram(m_program);

  // get uniforms
  GLint active_uniforms;
  glGetProgramiv(m_program, GL_ACTIVE_UNIFORMS, &active_uniforms);
  GLint max_length;
  glGetProgramiv(m_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);

  // this would also work before linking
  for (GLint u = 0; u < active_uniforms; u++) {
    GLchar str[max_length];
    GLint size;
    GLenum type;
    glGetActiveUniform(m_program, u, max_length, NULL, &size, &type, str);
    LOG_DEBUG("uniform name: \"" + str + "\" size: " + std::to_string(size) + " type: " + std::to_string(type))
    uniforms.emplace(std::make_pair(std::string(str), Uniform(u, size, type)));
  }

  return checkLinkStatus();
}

GLint gl::Shader::location(const std::string& name) { return glGetUniformLocation(m_program, name.c_str()); }

GLint gl::Shader::attrib_location(const std::string& name) { return glGetAttribLocation(m_program, name.c_str()); }
