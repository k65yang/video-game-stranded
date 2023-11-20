#version 330

uniform vec2 playerPos;
in vec3 in_position;

out vec2 texcoord;
out float distanceToPlayer;


void main()
{
	distanceToPlayer = length(playerPos - in_position.xy);
	
    gl_Position = vec4(in_position.xy, 0, 1.0);
	texcoord = (in_position.xy + 1) / 2.f;
	
}
