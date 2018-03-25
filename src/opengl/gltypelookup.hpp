#pragma once

#include <epoxy/gl.h>
#include <stdexcept>
#include <type_traits>
#include "boost/variant.hpp"

// lookup table for the different GL types

namespace gl {

bool constexpr contains(GLenum value) {
  (void)value;
  return false;
}

template <typename... Args> bool constexpr contains(GLenum value, GLenum head, Args... tail) { return value == head || contains(value, tail...); }

template <typename T> struct permissible_uniform_types {};

#define UNIFORM_TYPE(CPPTYPE, ...)                                                                                                                       \
  template <> struct permissible_uniform_types<CPPTYPE> {                                                                                                \
    static constexpr bool contains(GLenum value) { return ::gl::contains(value, __VA_ARGS__); }                                                          \
  };

UNIFORM_TYPE(GLfloat, GL_FLOAT, GL_BOOL, GL_FLOAT_VEC2, GL_BOOL_VEC2, GL_FLOAT_VEC3, GL_BOOL_VEC3, GL_FLOAT_VEC4, GL_BOOL_VEC4, GL_FLOAT_MAT2,
             GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_FLOAT_MAT2x3, GL_FLOAT_MAT2x4, GL_FLOAT_MAT3x2, GL_FLOAT_MAT3x4, GL_FLOAT_MAT4x2, GL_FLOAT_MAT4x3)

UNIFORM_TYPE(GLdouble, GL_DOUBLE, GL_DOUBLE_VEC2, GL_DOUBLE_VEC3, GL_DOUBLE_VEC4, GL_DOUBLE_MAT2, GL_DOUBLE_MAT3, GL_DOUBLE_MAT4, GL_DOUBLE_MAT2x3,
             GL_DOUBLE_MAT2x4, GL_DOUBLE_MAT3x2, GL_DOUBLE_MAT3x4, GL_DOUBLE_MAT4x2, GL_DOUBLE_MAT4x3)

UNIFORM_TYPE(GLint, GL_INT, GL_SAMPLER_1D, GL_SAMPLER_2D, GL_SAMPLER_3D, GL_SAMPLER_CUBE, GL_SAMPLER_1D_SHADOW, GL_SAMPLER_2D_SHADOW, GL_SAMPLER_1D_ARRAY,
             GL_SAMPLER_2D_ARRAY, GL_SAMPLER_1D_ARRAY_SHADOW, GL_SAMPLER_2D_ARRAY_SHADOW, GL_SAMPLER_2D_MULTISAMPLE, GL_SAMPLER_2D_MULTISAMPLE_ARRAY,
             GL_SAMPLER_CUBE_SHADOW, GL_SAMPLER_BUFFER, GL_SAMPLER_2D_RECT, GL_SAMPLER_2D_RECT_SHADOW, GL_INT_SAMPLER_1D, GL_INT_SAMPLER_2D,
             GL_INT_SAMPLER_3D, GL_INT_SAMPLER_CUBE, GL_INT_SAMPLER_1D_ARRAY, GL_INT_SAMPLER_2D_ARRAY, GL_INT_SAMPLER_2D_MULTISAMPLE,
             GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_INT_SAMPLER_BUFFER, GL_INT_SAMPLER_2D_RECT, GL_UNSIGNED_INT_SAMPLER_1D, GL_UNSIGNED_INT_SAMPLER_2D,
             GL_UNSIGNED_INT_SAMPLER_3D, GL_UNSIGNED_INT_SAMPLER_CUBE, GL_UNSIGNED_INT_SAMPLER_1D_ARRAY, GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,
             GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_UNSIGNED_INT_SAMPLER_BUFFER,
             GL_UNSIGNED_INT_SAMPLER_2D_RECT, GL_BOOL, GL_INT_VEC2, GL_BOOL_VEC2, GL_INT_VEC3, GL_BOOL_VEC3, GL_INT_VEC4, GL_BOOL_VEC4)

UNIFORM_TYPE(GLuint, GL_UNSIGNED_INT, GL_BOOL, GL_UNSIGNED_INT_VEC2, GL_BOOL_VEC2, GL_UNSIGNED_INT_VEC3, GL_BOOL_VEC3, GL_UNSIGNED_INT_VEC4, GL_BOOL_VEC4)

template <typename T> bool constexpr valid_uniform_type(GLenum value) { return permissible_uniform_types<T>::contains(value); }

typedef decltype(&glUniform1fv) uniform_float_method;
typedef decltype(&glUniform1dv) uniform_double_method;
typedef decltype(&glUniform1iv) uniform_int_method;
typedef decltype(&glUniform1uiv) uniform_uint_method;
typedef decltype(&glUniformMatrix2fv) uniform_matrix_float_method;
typedef decltype(&glUniformMatrix2dv) uniform_matrix_double_method;
typedef boost::variant<uniform_float_method, uniform_double_method, uniform_int_method, uniform_uint_method, uniform_matrix_float_method,
  uniform_matrix_double_method>
  uniform_method;

template <typename T> struct uniform_type_lookup {};

template <> struct uniform_type_lookup<GLfloat> { typedef uniform_float_method type; };
template <> struct uniform_type_lookup<GLdouble> { typedef uniform_double_method type; };
template <> struct uniform_type_lookup<GLint> { typedef uniform_int_method type; };
template <> struct uniform_type_lookup<GLuint> { typedef uniform_uint_method type; };

template <typename T> struct uniform_matrix_type_lookup {};

template <> struct uniform_matrix_type_lookup<GLfloat> { typedef uniform_matrix_float_method type; };
template <> struct uniform_matrix_type_lookup<GLdouble> { typedef uniform_matrix_double_method type; };

inline uniform_method getUniformMethod(const GLenum gltype) {
  switch (gltype) {
#define GL_UNIFORM_METHOD(UNIFORM, GLTYPE)                                                                                                                  \
  case GLTYPE:                                                                                                                                              \
    return &UNIFORM;
#define GL_UNIFORM_MATRIX_METHOD(UNIFORM, GLTYPE)                                                                                                           \
  case GLTYPE:                                                                                                                                              \
    return &UNIFORM;

    GL_UNIFORM_METHOD(glUniform1fv, GL_FLOAT)
    GL_UNIFORM_METHOD(glUniform2fv, GL_FLOAT_VEC2)
    GL_UNIFORM_METHOD(glUniform3fv, GL_FLOAT_VEC3)
    GL_UNIFORM_METHOD(glUniform4fv, GL_FLOAT_VEC4)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix2fv, GL_FLOAT_MAT2)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix3fv, GL_FLOAT_MAT3)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix4fv, GL_FLOAT_MAT4)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix2x3fv, GL_FLOAT_MAT2x3)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix2x4fv, GL_FLOAT_MAT2x4)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix3x2fv, GL_FLOAT_MAT3x2)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix3x4fv, GL_FLOAT_MAT3x4)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix4x2fv, GL_FLOAT_MAT4x2)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix4x3fv, GL_FLOAT_MAT4x3)

    GL_UNIFORM_METHOD(glUniform1dv, GL_DOUBLE)
    GL_UNIFORM_METHOD(glUniform2dv, GL_DOUBLE_VEC2)
    GL_UNIFORM_METHOD(glUniform3dv, GL_DOUBLE_VEC3)
    GL_UNIFORM_METHOD(glUniform4dv, GL_DOUBLE_VEC4)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix2dv, GL_DOUBLE_MAT2)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix3dv, GL_DOUBLE_MAT3)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix4dv, GL_DOUBLE_MAT4)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix2x3dv, GL_DOUBLE_MAT2x3)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix2x4dv, GL_DOUBLE_MAT2x4)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix3x2dv, GL_DOUBLE_MAT3x2)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix3x4dv, GL_DOUBLE_MAT3x4)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix4x2dv, GL_DOUBLE_MAT4x2)
    GL_UNIFORM_MATRIX_METHOD(glUniformMatrix4x3dv, GL_DOUBLE_MAT4x3)

    GL_UNIFORM_METHOD(glUniform1iv, GL_INT)
    GL_UNIFORM_METHOD(glUniform2iv, GL_INT_VEC2)
    GL_UNIFORM_METHOD(glUniform3iv, GL_INT_VEC3)
    GL_UNIFORM_METHOD(glUniform4iv, GL_INT_VEC4)
    GL_UNIFORM_METHOD(glUniform1iv, GL_BOOL)
    GL_UNIFORM_METHOD(glUniform2iv, GL_BOOL_VEC2)
    GL_UNIFORM_METHOD(glUniform3iv, GL_BOOL_VEC3)
    GL_UNIFORM_METHOD(glUniform4iv, GL_BOOL_VEC4)

    GL_UNIFORM_METHOD(glUniform1uiv, GL_UNSIGNED_INT)
    GL_UNIFORM_METHOD(glUniform2uiv, GL_UNSIGNED_INT_VEC2)
    GL_UNIFORM_METHOD(glUniform3uiv, GL_UNSIGNED_INT_VEC3)
    GL_UNIFORM_METHOD(glUniform4uiv, GL_UNSIGNED_INT_VEC4)
// samplers
    default:
      return &glUniform1iv;
  }
}

}
