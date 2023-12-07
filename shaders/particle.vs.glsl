#version 330

// Input attributes
layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_position;
layout(location = 3) in mat3 modelMatrix;

out vec3 vcolor;
out vec2 vpos;

// Application data
uniform mat3 view;
uniform mat3 projection;

void main()
{
	vpos = in_position.xy; // local coordinated before transform
	vcolor = in_color;
	vec3 pos = projection * view * modelMatrix * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}