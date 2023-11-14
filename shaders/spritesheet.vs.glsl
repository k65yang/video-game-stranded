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


uniform int framex;  // Player's frame X
uniform int framey;  // Player's frame Y
uniform int mframex; // Mob's frame X
uniform int mframey; // Mob's frame Y

uniform int isPlayer; // Flag to indicate if it's the player (1) 



void main()
{
    texcoord = in_texcoord;

    if (isPlayer == 1) {
        // Apply player-specific frame calculations
        texcoord.x += framex * 0.125f; //player_frame_w 
        texcoord.y += framey * 0.2f; //player_frame_
    } else {
        // Apply mob-specific frame calculations
        texcoord.x += mframex * 0.142857f; //mob_frame_w 
        texcoord.y += mframey * 0.25f; //mob_frame_h
    }

    vec3 pos = projection * view * transform * vec3(in_position.xy, 1.0);
    gl_Position = vec4(pos.xy, in_position.z, 1.0);
}