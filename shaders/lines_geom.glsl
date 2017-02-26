#version 430

// todo: draw thick lines with 3D effects

layout (lines) in;
layout (line_strip, max_vertices = 2) out;

in int          V_ID[];
in float        V_alpha[];

out vec4        color_val;
out float       alpha;

uniform int     y_axis;
uniform int     x_axis;

struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
    vec2 layout1;
    vec2 layout2;
};

layout (std430, binding=2) buffer mesh_data
{
    SSBO_datum SSBO_data[];
};

struct properties {
    vec2 pos_alpha;
    vec2 trans_alpha;
    vec2 color_alpha;
    vec2 point_size;
    vec2 interval;
    vec2 positions;
    vec4 render_type; // mesh triangles, mesh points, points skeleton, graph (points, edges)
    vec4 extra_info;  // x: axis type, y: connectivity graph, z, w: empty slots
};

struct ast_neu_properties {
    properties ast;
    properties neu;
};


layout (std430, binding=3) buffer space2d_data
{
    ast_neu_properties space2d;
};

void main() {
    int ID = V_ID[0];
    int type = int(SSBO_data[ID].center.w); // 0: astrocyte, 1: neurite

    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

    vec4 render_type = space_properties.render_type; // additional info
    vec4 extra_info = space_properties.extra_info; // additional info

    if ( extra_info.y == 0  ) {
        return;
    }

    color_val = vec4(1.0, 0, 1, 1);
    alpha = V_alpha[0];
    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = gl_in[0].gl_PointSize;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    gl_PointSize = gl_in[1].gl_PointSize;
    EmitVertex();
    EndPrimitive();


}
