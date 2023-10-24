#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform int light_up;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));

	// Make the screen red when the player takes damage
	if (light_up == 1)
	{
		color.xyz += (0.4) * vec3(1.0, 0.0, 0.0);
	}
}