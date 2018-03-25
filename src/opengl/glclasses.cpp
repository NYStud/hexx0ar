#include "glclasses.hpp"

gl::VAO::VAO() {}

gl::VAO::~VAO() { deinit(); }

void gl::VAO::init() { glGenVertexArrays(1, &m_name); bind(); }

void gl::VAO::deinit() { glDeleteVertexArrays(1, &m_name); }

void gl::VAO::bind() { glBindVertexArray(m_name); }

void gl::VAO::unbind() { glBindVertexArray(0); }

void gl::VAO::fill(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer) {
  glEnableVertexAttribArray(index);
  glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void gl::VAO::attachVBO(GLuint idx, GLuint vbo, GLintptr offset, GLsizei stride, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) {
  glEnableVertexArrayAttrib(m_name, idx);
  glVertexArrayVertexBuffer(m_name, idx, vbo, offset, stride);
  glVertexArrayAttribFormat(m_name, idx, size, type, normalized, relativeoffset);
  glVertexArrayAttribBinding(m_name, idx, idx);
}

void gl::VAO::attachIBO(GLuint ibo) {
  glVertexArrayElementBuffer(m_name, ibo);
}

void gl::VAO::divisor(GLuint idx, GLuint divisor) {
  glVertexArrayBindingDivisor(m_name, idx, divisor);
}

gl::Texture::Texture() {}

gl::Texture::~Texture() { deinit(); }

void gl::Texture::init() { glGenTextures(1, &m_name); bind(); }

void gl::Texture::deinit() { glDeleteTextures(1, &m_name); }

void gl::Texture::bind() { glBindTexture(GL_TEXTURE_2D, m_name); }

void gl::Texture::activate(GLenum slot) {
  glActiveTexture(slot);
  bind();
}

void gl::Texture::unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

void gl::Texture::fill(GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* data) {
  bind();
  m_width = width;
  m_height = height;
  m_format = format;
  m_internal_format = internalFormat;
  m_type = type;
  glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, border, format, type, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

gl::Framebuffer::Framebuffer() {}

gl::Framebuffer::~Framebuffer() { deinit(); }

void gl::Framebuffer::init() { glGenFramebuffers(1, &m_name); bind(); }

void gl::Framebuffer::deinit() { glDeleteFramebuffers(1, &m_name); }

void gl::Framebuffer::bind() { glBindFramebuffer(GL_FRAMEBUFFER, m_name); }

void gl::Framebuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void gl::Framebuffer::attachRenderbuffer(GLenum attachment, GLuint rbo) {
  bind();
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rbo);
}

void gl::Framebuffer::attachTexture(GLenum attachment, GLuint tex) {
  bind();
  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex, 0);
}

bool gl::Framebuffer::check() { return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE; }

gl::Renderbuffer::Renderbuffer() {}

gl::Renderbuffer::~Renderbuffer() { deinit(); }

void gl::Renderbuffer::init() { glGenRenderbuffers(1, &m_name); bind(); }

void gl::Renderbuffer::deinit() { glDeleteRenderbuffers(1, &m_name); }

void gl::Renderbuffer::bind() { glBindRenderbuffer(GL_RENDERBUFFER, m_name); }

void gl::Renderbuffer::unbind() { glBindRenderbuffer(GL_RENDERBUFFER, 0); }

void gl::Renderbuffer::create(GLenum internalformat, GLsizei width, GLsizei height) {
  bind();
  glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
}

gl::RenderToTexture::RenderToTexture(GLenum attachment) {
  m_tex = Texture();
  m_attachment = attachment;
}

void gl::RenderToTexture::init(GLint w, GLint h) {
  m_tex.init();
  m_tex.fill(0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  m_rbo.init();
  m_rbo.bind();
  m_rbo.create(GL_DEPTH_COMPONENT, w, h);
  m_fbo.init();
  m_fbo.bind();
  m_fbo.attachRenderbuffer(GL_DEPTH_ATTACHMENT, m_rbo.id());
  m_fbo.attachTexture(m_attachment, m_tex.id());
}

void gl::RenderToTexture::deinit() {
  m_fbo.deinit();
  m_rbo.deinit();
  m_tex.deinit();
}

void gl::RenderToTexture::bind() { m_fbo.bind(); }

void gl::RenderToTexture::unbind() { m_fbo.unbind(); }

GLuint gl::RenderToTexture::getTexture() { return m_tex.id(); }
