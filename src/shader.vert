#version 120

// Points to position_data.
attribute vec2 position;

uniform float screen_ratio;

void main() 
{
    vec2 u = 0.99*position;
    gl_Position = vec4(u.x/screen_ratio, u.y, 0.0, 1.0);
}
