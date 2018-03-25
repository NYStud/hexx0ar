#version 330

#extension GL_ARB_gpu_shader5 : require
#extension GL_ARB_draw_instanced : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shading_language_420pack: require

layout (location = 0) in vec2 in_vertex;
layout (location = 1) in vec4 in_color;
layout (location = 2) in vec2 in_uv;

out vec2 vertex;
out vec4 color;
out vec2 uv;

uniform mat4 mvp;

void main() {
     vertex = in_vertex;
     color = in_color;
     uv = in_uv;

     gl_Position.xy = in_vertex;
     gl_Position.zw = vec2(0.0f, 1.0);

     //rotate in z axis data.z
     //scale x/y with data.w

     gl_Position = mvp * gl_Position;
}
