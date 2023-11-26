#version 450


layout(location = 0) out vec3 a_Color;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

float rand(vec2 co){
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main() 
{
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);

    //a_Color = colors[gl_VertexIndex];

    float randomIndex = rand(vec2(float(gl_VertexIndex), 0.0));
    int colorIndex = int(mod(randomIndex * 3.0, 3.0));
    a_Color = colors[colorIndex];
}