#version 330

// From vertex shader
in vec2 texcoord;
flat in uint tex_i;
//out uint flags;

// Application data
//uniform sampler2D textures[11];
uniform vec3 fcolor;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	// TODO: get textures working
	color = vec4(fcolor, tex_i / 11.0); //texture(textures[tex_i], texcoord);
}
