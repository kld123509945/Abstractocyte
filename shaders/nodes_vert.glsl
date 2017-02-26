#version 430

// this is for neurites nodes at the most abstract point.
// so only when the curser is at (100, 100) the node of this will show uo
// do I need this? only for the edges

// in: per vertex data
layout (location = 0) in int ID;

out int V_ID;
out float V_alpha;

// World transformation
uniform mat4 mMatrix;
uniform mat4 m_noRartionMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;
uniform int is2D;
uniform int  y_axis;
uniform int  x_axis;

struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
    vec2 layout1;   // neurite nodes
    vec2 layout2;   // neurites nodes + astrocyte skeleton
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

void main(void)
{
    // two positions to interpolate between:
    // 3D with rotation
    // projected 2D without rotation
    mat4 m_noRotvpMatrix = pMatrix * vMatrix * mMatrix;

    // todo: interpolate between different layouts based on state
    vec4 node_layout1 = m_noRotvpMatrix * vec4(SSBO_data[ID].layout1, 0, 1);
    vec4 node_layout2 = m_noRotvpMatrix * vec4(SSBO_data[ID].layout2, 0, 1);
    float position_intp = translate(y_axis, 80, 100, 0, 1);

    V_alpha =  translate(y_axis, 80, 100,  0, 1);
    V_alpha = 1;
    vec4 node_pos  = mix(node_layout2, node_layout1 ,  position_intp);

    gl_Position =  node_pos; // original position
    gl_PointSize = 20;
    V_ID = ID;
}
