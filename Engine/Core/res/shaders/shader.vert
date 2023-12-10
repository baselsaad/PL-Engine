#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;

layout(push_constant) uniform PushConstants {
    mat4 u_MVP;
} pc;

layout(location = 0) out vec3 v_FragColor;

void main() {
    gl_Position = pc.u_MVP * vec4(a_Position, 1.0);
    v_FragColor = a_Color;
}
