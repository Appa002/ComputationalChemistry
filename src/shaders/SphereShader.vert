#version 420

layout(location = 0) in vec3 vertexPosition;

out vec4 fragmentWorldPos;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(vertexPosition, 1.0);

    fragmentWorldPos = gl_Position;
}
