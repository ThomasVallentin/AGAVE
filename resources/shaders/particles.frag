#version 460 core


in VertexData
{
    vec3 position;
    vec3 velocity;
} inVertex;


out vec4 fColor;


void main() 
{
    fColor = vec4(mix(vec3(0, 0, 1.0), vec3(1.0, 0.0, 0.0), min(1, length(inVertex.velocity))), 
                  1.0);
}
