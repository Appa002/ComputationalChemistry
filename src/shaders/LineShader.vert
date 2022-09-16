#version 420

layout(location = 0) in vec3 mesh;


uniform mat4 mvp;

void main() {
    gl_Position = mvp * vec4(mesh, 1.0f);
    //gl_Position = vec4(mesh, 1.0f);
}

