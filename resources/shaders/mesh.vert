#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in mat4 aMatrix;
layout(location = 7) in vec4 aColor;

uniform mat4 uViewMatrix;
uniform mat4 uProjMatrix;

out VertexData
{
    vec3 position;
    vec3 normal;
    vec2 texCoord;
    vec4 color;
} outVertex;


void main() 
{
    vec4 position = uProjMatrix * uViewMatrix * aMatrix * vec4(aPosition, 1.0);

    outVertex.position = position.xyz;
    outVertex.normal = vec3(uViewMatrix * aMatrix * vec4(aNormal, 0.0));
    outVertex.texCoord = aTexCoord;
    outVertex.color = aColor;

    gl_Position = position;
}
