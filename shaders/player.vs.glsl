#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 view;
uniform mat3 projection;

uniform int framex; 
uniform int framey; 

void main()
{
	texcoord = in_texcoord;
	    // Reflect both on X and Y axes
    //texcoord = vec2(texcoord.x, 1.0 - texcoord.y);
	// frame_h and frame_w is applied with 0.2 and 0.125
	texcoord.x += framex *  0.125f; 
	texcoord.y += framey* 0.2f;
	vec3 pos = projection * view * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}