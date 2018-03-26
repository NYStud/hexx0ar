#version 330

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
