#version 330

#extension GL_ARB_gpu_shader5 : require
#extension GL_ARB_draw_instanced : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shading_language_420pack: require

uniform sampler2D tex0;

in vec2 vertex;
in vec4 color;
in vec2 uv;

out vec4 out_color;

void main()
{
  out_color = texture(tex0, vec2(uv.s, uv.t));

  if(out_color.a < 0.1f)
    discard;

  out_color *= color;
}
