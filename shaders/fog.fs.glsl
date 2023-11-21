#version 330

uniform sampler2D screen_texture;
uniform float fow_darken_factor;
uniform float fowRadius;
uniform int enableFow;

in vec2 texcoord;


layout(location = 0) out vec4 color;


void main()
{
	
	float magnifier = 3.f;
	float distanceScaling = 18.f;

	// calculate distance between center pixel and current pixel
	float disToFOW = distance(texcoord, vec2(0.5f, 0.5f));

	// texture from frame buffer
    vec4 in_color = texture(screen_texture, texcoord);

	// referece: drawing circle with distance in glsl reference: https://www.youtube.com/watch?v=L-BA4nJJ8bQ

	// For pixel within fow, adjust color based on the distance of pixel to center pixel. Else, apply a darken factor on top of current pixel color
	if (enableFow == 1) {
		if (disToFOW < (fowRadius / distanceScaling)) {
			color = (1 - magnifier * disToFOW ) * in_color;
		} else {
	// for pixel outside fog, darken but keep them still visible
			color = in_color * fow_darken_factor;
		}
	
	} else {
	// fog disabled
		color = in_color;

	}
	
}

