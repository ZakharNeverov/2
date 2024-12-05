#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragPos;
out vec3 normalInterp;
out vec2 texCoordInterp;

void main()
{
    vec4 worldPosition = modelMatrix * vec4(position, 1.0);
    fragPos = worldPosition.xyz;
    normalInterp = mat3(transpose(inverse(modelMatrix))) * normal;
    texCoordInterp = texCoord;
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}
