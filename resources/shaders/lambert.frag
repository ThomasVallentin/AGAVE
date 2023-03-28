#version 460 core

in VertexData
{
    vec3 position;
    vec3 normal;
    vec2 texCoord;
    vec4 color;
} inVertex;


out vec4 fColor;


void main() 
{
    vec3 normal = normalize(inVertex.normal);
    vec3 toCamera = normalize(-inVertex.position);
    fColor = vec4(inVertex.color.rgb, inVertex.color.a * mix(1.0, 0.75, abs(dot(normal, toCamera))));
}
