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

uniform vec2 spriteFrame; //each of the sprite frame
uniform vec2 frameDimensions;  // x: frame width, y: frame height


//to pass in the animation type the and the period of update as input parameters
void main()
{
    texcoord = in_texcoord;
    texcoord.x += spriteFrame.x* frameDimensions.x;
    texcoord.y += spriteFrame.y* frameDimensions.y;

    vec3 pos = projection * view * transform * vec3(in_position.xy, 1.0);
    gl_Position = vec4(pos.xy, in_position.z, 1.0);
}