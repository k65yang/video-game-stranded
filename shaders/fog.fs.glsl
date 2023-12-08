#version 330

uniform sampler2D screen_texture;
uniform float fow_darken_factor;
uniform float scaled_down_fowRadius;
uniform int enableFow;
uniform ivec2 aspect_ratio;

in vec2 texcoord;


layout(location = 0) out vec4 color;


void main()
{
	// get texture from frame buffer
	vec4 in_color = texture(screen_texture, texcoord);

	if (enableFow == 1) {
		float magnifier = 3.f;

		vec2 f_aspect_ratio = vec2(aspect_ratio);
		vec2 scaling = f_aspect_ratio.xy / f_aspect_ratio.yx;

		// We want to scale the apparent distanced from the centre.
		// The bigger ratio will be used as the baseline distance.
		// The smaller side of the screen will be scaled up.
		if (aspect_ratio.x > aspect_ratio.y) {
			scaling.x = 1;
		}
		else {
			scaling.y = 1;
		}

		// calculate distance between center pixel and current pixel
		float disToFOW = distance(texcoord * scaling, vec2(0.5f , 0.5f) * scaling);

		// referece: drawing circle with distance in glsl reference: https://www.youtube.com/watch?v=L-BA4nJJ8bQ

		// For pixel within fow, adjust color based on the distance of pixel to center pixel. Else, apply a darken factor on top of current pixel color
		if (disToFOW < scaled_down_fowRadius) {
			
			color =  clamp((1 - 0.754 * (disToFOW / scaled_down_fowRadius)), 0.0f , 1.0f) * in_color;
		} else{
		// for pixel outside fog, darken but keep them still visible
			color = in_color * fow_darken_factor;
		}
	
	} else {
	// fog disabled
		color = in_color;

	}
	
}

