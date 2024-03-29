#version 450

// Vertex Attributes 
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec4 a_ObjectID;

layout(push_constant) uniform PushConstants 
{
    mat4 u_MVP;
} pc;

// Outputs to fragment shader
layout(location = 0) out vec4 v_FragColor;
layout(location = 1) out flat vec4 v_ObjectID;
//-----------------------------------------------------------------------------------------------------------------------------------------------

mat4 CreateTranslationMatrix(vec3 translation) 
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(translation, 1.0)
    );
}

mat4 CreateScaleMatrix(vec3 scale) 
{
    //scale from orgin
    return mat4(
        vec4(scale.x, 0.0, 0.0, 0.0),
        vec4(0.0, scale.y, 0.0, 0.0),
        vec4(0.0, 0.0, scale.z, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
}

void main() 
{
    // mat4 translationMatrix = CreateTranslationMatrix(a_Translation);
    // mat4 scaleMatrix = CreateScaleMatrix(a_Scale);
    // mat4 transformMatrix =  translationMatrix * scaleMatrix; // order, scale first
    //gl_Position = pc.u_MVP * (transformMatrix * vec4(a_Position, 1.0) );

    gl_Position = pc.u_MVP * vec4(a_Position, 1.0);
    v_FragColor = a_Color;
    v_ObjectID = a_ObjectID;
}
