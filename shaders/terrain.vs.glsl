#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;
in uint in_tex_i;
in uint in_flags;

// Passed to fragment shader
out vec2 texcoord;
flat out uint tex_i;
//out uint flags;

// Application data
uniform mat3 viewMatrix;
uniform mat3 projectionMatrix;
uniform vec2 uv_offsets;
uniform vec2 texel_offsets;

vec2 directional_uv() {
	return uv_offsets + texel_offsets;
}

void main()
{
	if (in_flags != 1u) {
		texcoord = in_texcoord;
	}
	else {
		texcoord = directional_uv();
	}
	tex_i = in_tex_i;

	vec3 pos = projectionMatrix * viewMatrix * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}