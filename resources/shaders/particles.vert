#version 460 core

layout(location = 0) in vec3 aPosition;
// layout(location = 1) in vec3 aVelocity;

uniform mat4 uViewProjMatrix;

out VertexData
{
    vec3 position;
    vec3 velocity;
} outVertex;


void main() 
{
    vec4 position = uViewProjMatrix * vec4(aPosition, 1.0);
    outVertex.position = position.xyz;
    outVertex.velocity = vec3(1, 1, 1);

    gl_Position = position;
    gl_PointSize = 5;
}
