#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(push_constant) uniform PushConstants {
    mat4 transformationMatrix;
} pc;

layout(location = 0) out vec3 v_FragColor;

void main() {
    gl_Position = pc.transformationMatrix * vec4(inPosition, 1.0);

    v_FragColor = inColor;
}
