#version 330

uniform sampler2D screen_texture;
uniform float fow_darken_factor;
uniform float fowRadius;
uniform int enableFow;

in vec2 texcoord;


layout(location = 0) out vec4 color;

// reference: drawing ellipse in shader https://www.shadertoy.com/view/wdKXzt
// reference: ellipse equation https://www.khanacademy.org/math/precalculus/x9e81a4f98389efdf:conics/x9e81a4f98389efdf:ellipse-center-radii/v/ellipse-standard-equation-from-graph

// To accomodate scaling due to aspect ratio, fog of war shape is now an ellipse.
void main()
{
	
	float magnifier = 3.f;
	float distanceScaling = 90.f;
	float darkening = 0.75;

	// horizontal distance. 16/9 is our aspect ratio
	float a = 0.55 * 16/9;

	// vertical distance
    float b = 1.3;
    
    float x = texcoord.x - 0.5;
    float y = texcoord.y - 0.5;
    
    // calculate distance between point and fow ellipse
	float dis = pow( x, 2.0 ) / ( a * a ) + pow( y, 2.0 ) / ( b * b );

	// texture from frame buffer
    vec4 in_color = texture(screen_texture, texcoord);

    if (enableFow == 1) {
		
		if ( dis <= (fowRadius / distanceScaling) )
		{
		// within fow
			color = darkening * (1 - magnifier * dis) * in_color;    
		}
		else
		{
     		color = in_color * fow_darken_factor;
		}
	} else {
	// fog disabled
		color = in_color;

	}

}

/*
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

*/