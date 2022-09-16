#version 420

layout(location = 0) in vec3 mesh;
uniform mat4 mvp;

out vec3 worldPos;

void main() {
    gl_Position = mvp * vec4(mesh, 1.0f);
    worldPos = mesh;
    //gl_Position = vec4(mesh, 1.0f);
}

