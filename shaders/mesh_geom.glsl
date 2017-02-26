#version 430

in  int         V_ID[];

in vec4         V_color_val[];
in float        V_alpha[];
in float        V_color_intp[];

out float       color_intp;
out vec4        color_val;
out vec3        normal_out;
out float       alpha;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

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
    vec4 extra_info;  // x: axis type, y, z, w: empty slots
};

struct ast_neu_properties {
    properties ast;
    properties neu;
};


layout (std430, binding=3) buffer space2d_data
{
    ast_neu_properties space2d;
};

float translate(float value, float leftMin, float leftMax, float rightMin, float rightMax)
{
    // if value < leftMin -> value = leftMin
    value = max(value, leftMin);
    // if value > leftMax -> value = leftMax
    value = min(value, leftMax);
    // Figure out how 'wide' each range is
    float leftSpan = leftMax - leftMin;
    float rightSpan = rightMax - rightMin;

    // Convert the left range into a 0-1 range (float)
    float valueScaled = float(value - leftMin) / float(leftSpan);

    // Convert the 0-1 range into a value in the right range.
    return rightMin + (valueScaled * rightSpan);
}

void main() {
  vec3 A = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 B = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  normal_out = normalize(cross(A,B));

  for(int i = 0; i < 3; i++) {
    int ID = V_ID[i];
    int type = int(SSBO_data[ID].center.w);
    color_val = V_color_val[i];

    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

    vec4 render_type = space_properties.render_type; // additional info

    if (render_type.x == 0) {
        return;
    }

    alpha =  V_alpha[i];

    color_intp =  V_color_intp[i];

    gl_Position = gl_in[i].gl_Position;
    EmitVertex();
  }

  EndPrimitive();
}


