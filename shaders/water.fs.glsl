#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float screen_darken_factor;
uniform float fogRadius;

in vec2 texcoord;


layout(location = 0) out vec4 color;


void main()
{
	
	float darkenOutsideFow = 0.25;
	float magnifier = 3.f;
	float distanceScaling = 18.f;

	// calculate distance between center pixel and current pixel
	float disToFOW = distance(texcoord, vec2(0.5f, 0.5f));
	//disToFOW  = smoothstep(0.8, 0.3, 1.0 - disToFOW);


	// texture from frame buffer
    vec4 in_color = texture(screen_texture, texcoord);

	// drawing circle with distance in glsl reference: https://www.youtube.com/watch?v=L-BA4nJJ8bQ
	// change pixel color based on distance to center pixel, if further, the texture color is darker
	if (disToFOW < (fogRadius / distanceScaling)) {
		color = (1 - magnifier * disToFOW ) * in_color;
	} else {
	// for pixel outside fog, darken but keep them still visible
		color = in_color * darkenOutsideFow;
	}
	
	// comment above and uncomment this line to disable fow
	// color = in_color;
}



/*
vec2 distort(vec2 uv) 
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE THE WATER WAVE DISTORTION HERE (you may want to try sin/cos)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	return uv;
}

vec4 color_shift(vec4 in_color) 
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE THE COLOR SHIFTING HERE (you may want to make it blue-ish)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	return in_color;
}

vec4 fade_color(vec4 in_color, float dis) 
{
	//if (screen_darken_factor > 0)
		//in_color -= screen_darken_factor * vec4(0.8, 0.8, 0.8, 0);
	
	if (dis < 7.f) {
		in_color -= 0 * vec4(0.8, 0.8, 0.8, 0);
	} else {
		in_color -= 0.5 * vec4(0.8, 0.8, 0.8, 0);
	}

	return in_color;
}
*/