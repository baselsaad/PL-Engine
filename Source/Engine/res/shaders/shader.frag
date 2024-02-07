#version 450

// vertex shader input
layout(location = 0) in vec4 v_FragColor;
//layout(location = 1) in flat vec4 v_ObjectID;

// output
layout(location = 0) out vec4 o_Color;
//layout(location = 1) out vec4 o_ObjectID;

void main() 
{
    o_Color = v_FragColor;
    //o_ObjectID = v_ObjectID;
}