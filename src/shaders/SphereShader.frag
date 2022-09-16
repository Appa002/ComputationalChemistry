#version 420

out vec4 outColour;

in vec4 fragmentWorldPos;
uniform vec3 atom_colour;

void main() {
    outColour = vec4(atom_colour.rgb, 1.0f);
}
