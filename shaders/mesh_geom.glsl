#version 430

#define astrocyte 6

in  int         V_ID[];

in vec4         V_color_val[];
in float        V_alpha[];
in float        V_color_intp[];
in vec4			V_worldPos[];
in vec4         V_normal[];
in vec3         V_fragTexCoord[];
in vec3			E_eye[];

out vec4        color_val;
out vec3        normal_out;
//out vec2		colorIntp_alpha;
out float		alpha;
out float		color_intp;
out vec3		vposition;
out float       G_ID;
out vec3        G_fragTexCoord;
out vec3		eye;
flat out int	otype;

uniform int     hoveredID;
uniform int     smooth_shading;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

struct SSBO_datum {
	vec4 color;
	vec4 center;
	vec4 info;
	vec2 layout1;
	vec2 layout2;
};

layout(std430, binding = 2) buffer mesh_data
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


layout(std430, binding = 3) buffer space2d_data
{
	ast_neu_properties space2d;
};


void main() {
    // if smooth shading is disabled
    if (smooth_shading == 0) {
        vec3 A = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
        vec3 B = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
        normal_out = normalize(cross(A,B));
    }

    for (int i = 0; i < 3; i++) {
        if (smooth_shading == 1)
            normal_out = V_normal[i].rgb;
        int ID = V_ID[i];
        G_ID =  float(ID);


        int filter_value = int(SSBO_data[ID].info.w);

        int visibility = (filter_value >> 0) & 1;
        if (visibility == 1)
            return;

        if (ID == 0)
            return;

        int type = int(SSBO_data[ID].center.w);
        color_val = V_color_val[i];
		otype = type;
        if (hoveredID == ID) {
            color_val += vec4(0.2, 0.2, 0.2, 0);
        }

        properties space_properties = (type == astrocyte) ? space2d.ast : space2d.neu;

        vec4 render_type = space_properties.render_type; // additional info

        if (render_type.x == 0)
            return;

		alpha = V_alpha[i];


		if (alpha < 0.05)
            return;


        eye = E_eye[i];

		color_intp = V_color_intp[i];
        vposition = V_worldPos[i].xyz;
        gl_Position = gl_in[i].gl_Position;
        G_fragTexCoord = V_fragTexCoord[i];

        EmitVertex();
    }

    EndPrimitive();
}
