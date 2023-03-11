#version 460 core


in VertexData
{
    vec3 position;
} inVertex;


out vec4 fColor;


void main() 
{
    fColor = vec4(1.0, 0.0, 0.0, 1.0);
}
