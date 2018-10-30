#version 120

// Helpers
float len2(vec2 a)
{
    return dot(a, a);
}

vec2 cconj(vec2 a)
{
    return vec2(a.x, -a.y);
}
vec2 cmul(vec2 a, vec2 b)
{
    return vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}
vec2 cdiv(vec2 a, vec2 b)
{
    return cmul(a, cconj(b))/len2(b);
}

vec2 S(vec2 a, vec2 x)
{
    return cdiv(x + a, vec2(1, 0) + cmul(cconj(a), x));
}


// Shader inputs.
// Points to position_data.
attribute vec2 position;

uniform float screen_ratio;
uniform float screen_zoom;
uniform vec2 pan;

void main() 
{
    vec2 y = S(pan, position);

    vec2 u = screen_zoom*y;
    gl_Position = vec4(u.x/screen_ratio, u.y, 0.0, 1.0);
}
