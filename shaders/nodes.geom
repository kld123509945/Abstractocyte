#version 430

layout (points) in;
layout (points, max_vertices = 1) out;


void main() {
    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = 20;
    EmitVertex();
    EndPrimitive();
}
