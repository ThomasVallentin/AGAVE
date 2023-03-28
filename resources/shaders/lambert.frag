#version 460 core

uniform vec4 uColor = vec4(1, 0, 0, 1);

in VertexData
{
    vec3 position;
    vec3 normal;
    vec2 texCoord;
} inVertex;


out vec4 fColor;


void main() 
{
    vec3 normal = normalize(inVertex.normal);
    vec3 toCamera = normalize(-inVertex.position);
    fColor = vec4(uColor.rgb, uColor.a * mix(1.0, 0.75, abs(dot(normal, toCamera))));
}
