#version 330 core

//[TODO] adapt the shader so it works with my opengl
//be able to pass radius, pos and color of the light wanted
//see if i can adapt it to take normal maps from tiles

out vec4 FragColor;

// Ordered Dithering table
const float threshold[16] = float[16](
    1./16., 9./16., 3./16., 11./16., 
    13./16., 5./16., 15./16., 7./16., 
    4./16., 12./16., 2./16., 10./16.,
    16./16., 8./16., 14./16., 6./16.);

//Table Lookup 
float findClosest(int x, int y, float v)
{
    return step(threshold[2*y+x],v);
}

//main
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    //vec4 col = texture(iChannel0, uv);

    // https://en.wikipedia.org/wiki/Ordered_dithering
   	int x = int(fragCoord.x) % 4;
    int y = int(fragCoord.y) % 4;
    
    //vec2 st = gl_FragCoord.xy/u_resolution;
    float pct = 0.0;

    // vec2 -> pos of the origin of the light 
    // a. The DISTANCE from the pixel to the center
    pct = 1.5 - distance(uv,vec2(0.500)) * (4.0 * (sin(iTime / 2.0) + 2.0));
    
    // https://en.wikipedia.org/wiki/Luma_(video)
	//float lum = dot(vec3(0.2126, 0.7152, 0.0722), col.rgb);
    //lum = findClosest(x,y, lum);
    
    float light = findClosest(x, y, pct);
    //float light = findClosest((x + y) / 2, (x + y) / 4, pct);
 
	// Output to screen
    if(light > 0.0)
        fragColor = vec4(vec3(0.0, 0.0, 1.0), 1.0);
    else
        fragColor = vec4(vec3(0.0), 1.0);
}