#version 330

// From vertex shader
in vec2 texcoord;
flat in uint tex_i;
//out uint flags;

// Application data
uniform sampler2DArray textures;
//uniform vec3 fcolor;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	vec3 _uv = vec3(texcoord, tex_i);
	color = texture(textures, _uv);
}
