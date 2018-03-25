/*
 * This file provides some useful opengl abstraction classes.
 *
 * It needs some new opengl features like GL_ARB_direct_state_access for some classes.
 * */

#pragma once

#include <epoxy/gl.h>
//#include <epoxy/glx.h>

#include "application/log.hpp"
#include <string>

namespace gl {

/*
 * this class abstracts a generic opengl buffer object.
 *
 * it can be templated with any buffer type enum in opengl.
 *
 * as every opengl object it's bound on initialization.
 * */
template <GLenum T> class BufferObject {
private:
  GLuint m_name;

protected:
public:
  BufferObject();

  ~BufferObject();

  void init();

  void deinit();

  void bind();

  void bind(GLuint index, GLintptr offset = 0, GLsizeiptr size = 0);

  void unbind();

  void fill(GLenum usage, GLsizei size = 0, const GLvoid* data = NULL);

  void subfill(GLintptr offset, GLsizei size, const GLvoid* data);

  void storage(GLsizeiptr size, const GLvoid * data, GLbitfield flags);

  GLvoid* map(GLenum access, GLintptr offset, GLsizei length);

  void unmap();

  GLuint id() { return m_name; }
};

/*
 * some standard buffer objects
 * */
typedef BufferObject<GL_ARRAY_BUFFER> VBO;
typedef BufferObject<GL_ELEMENT_ARRAY_BUFFER> IBO;
typedef BufferObject<GL_ELEMENT_ARRAY_BUFFER> EBO;
typedef BufferObject<GL_SHADER_STORAGE_BUFFER> SSBO;
typedef BufferObject<GL_UNIFORM_BUFFER> UBO;

/*
 * this class abstracts a single VAO.
 *
 * as every opengl object it's bound on initialization.
 * */
class VAO {
private:
  GLuint m_name;

protected:
public:
  VAO();

  ~VAO();

  void init();

  void deinit();

  void bind();

  void unbind();

  void fill(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride = 0, const GLvoid* pointer = NULL);

  void attachVBO(GLuint idx, GLuint vbo, GLintptr offset, GLsizei stride, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);

  void attachIBO(GLuint ibo);

  void divisor(GLuint idx, GLuint divisor);

  GLuint id() { return m_name; }
};

/*
 * very simple Texture wrapper.
 * */
class Texture {
private:
  GLuint m_name;
  GLsizei m_width;
  GLsizei m_height;
  GLint m_internal_format;
  GLenum m_format;
  GLenum m_type;

protected:
public:
  Texture();

  ~Texture();

  void init();

  void deinit();

  void bind();

  void activate(GLenum slot);

  void unbind();

  //this sets the internal variables, calls glTexImage2D
  //and sets the texparamers to default values GL_LINEAR (MIN/MAG filter) and GL_REPEAT (wrapping s/t)
  void fill(GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* data);

  GLuint id() {  return m_name; }
  GLsizei width() { return m_width; }
  GLsizei height() { return m_height; }
  GLint internal_format() { return m_internal_format; }
  GLenum format() { return m_format; }
  GLenum type() { return m_type; }
};

class Framebuffer {
private:
  GLuint m_name;

protected:
public:
  Framebuffer();

  ~Framebuffer();

  void init();

  void deinit();

  void bind();

  void unbind();

  void attachRenderbuffer(GLenum attachment, GLuint rbo);

  void attachTexture(GLenum attachment, GLuint tex);

  bool check();

  GLuint id() {  return m_name; };
};

class Renderbuffer {
private:
  GLuint m_name;

protected:
public:
  Renderbuffer();

  ~Renderbuffer();

  void init();

  void deinit();

  void bind();

  void unbind();

  void create(GLenum internalformat, GLsizei width, GLsizei height);

  GLuint id() {  return m_name; };
};

class RenderToTexture {
private:
  GLenum m_attachment;
  Texture m_tex;
  Framebuffer m_fbo;
  Renderbuffer m_rbo;

public:
  RenderToTexture(GLenum attachment);

  void init(GLint w, GLint h);

  void deinit();

  void bind();

  void unbind();

  GLuint getTexture();
};

/*
 * these struct can be used for glDrawArraysIndirect.
 * */
struct DrawArraysIndirectCommand {
  GLuint count;
  GLuint instanceCount;
  GLuint first;
  GLuint baseInstance;
};

/*
 * these struct can be used for glDrawElementsIndirect.
 * */
struct DrawElementsIndirectCommand {
  GLuint  count;
  GLuint  primCount;
  GLuint  firstIndex;
  GLuint  baseVertex;
  GLuint  baseInstance;
};

/*
  Implementation of the bufferobject
  with a little bit of preprocessor magic.
 */

#define TBufferObject_(pre, post) template <GLenum T> pre BufferObject<T>::post
#define TBufferObject(...) TBufferObject_(__VA_ARGS__)

TBufferObject(, BufferObject)() {}

TBufferObject(, ~BufferObject)() { deinit(); }

TBufferObject(void, init)() { glGenBuffers(1, &m_name); bind(); }

TBufferObject(void, deinit)() { glDeleteBuffers(1, &m_name); }

TBufferObject(void, bind)() { glBindBuffer(T, m_name); }

TBufferObject(void, bind)(GLuint index, GLintptr offset, GLsizeiptr size) {
  // todo
  (void)index;
  (void)offset;
  (void)size;
}

TBufferObject(void, unbind)() { glBindBuffer(T, 0); }

TBufferObject(void, fill)(GLenum usage, GLsizei size, const GLvoid* data) { glBufferData(T, size, data, usage); }

TBufferObject(void, subfill)(GLintptr offset, GLsizei size, const GLvoid* data) { glBufferSubData(T, offset, size, data); }

TBufferObject(void, storage)(GLsizeiptr size, const GLvoid * data, GLbitfield flags) { glNamedBufferStorage(m_name, size, data, flags); }

TBufferObject(GLvoid*, map)(GLenum access, GLintptr offset, GLsizei length) {
  return glMapNamedBufferRange(m_name, offset, length, access);
}

TBufferObject(void, unmap)() {
  glUnmapNamedBuffer(m_name);
}

}
