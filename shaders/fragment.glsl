#version 330 core

out vec4 fragColor;
in vec3 vertexColor;
uniform float brightness;

void main() {
    fragColor = vec4(vertexColor * brightness, 1.0);
}