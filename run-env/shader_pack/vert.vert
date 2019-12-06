#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragColor;

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 inCol;
//layout(location = 2) in vec4 arbtest1;
//layout(location = 3) in float arbtest2;

void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
    fragColor = inCol;
}
