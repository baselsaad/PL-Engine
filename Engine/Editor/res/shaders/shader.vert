#version 450

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 v_FragColor;

void main() {
    gl_Position = inPosition;
    v_FragColor = inColor;
}
