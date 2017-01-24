#version 430

in vec4         Gskeleton_vx;
in vec3         normal_out;
in float        G_ID;

in float        alpha;
in float        color_intp;
in vec4         color_val;
out vec4        outcol;

float Ns = 250;
vec4 mat_specular = vec4(1);
vec4 light_specular = vec4(1);

void main() {
        vec3 lightDir =  vec3(0.5, 0.0, -0.9);
        vec3 normal;
        normal.xy = gl_PointCoord * 2.0 - vec2(1.0);
        float mag = dot(normal.xy, normal.xy);
        if (mag > 1.0) discard;
        normal.z = -sqrt(1.0-mag);
        normal = normalize(normal);
        float diffuse = clamp( dot(normal, lightDir), 0, 1);
        vec3 eye = vec3(0.0, 0.0, 1.0);
        vec3 halfVector = normalize( eye + lightDir );
        float spec = max( pow( dot(normal, halfVector), Ns ), 0. );
        vec4 S = light_specular * mat_specular * spec;
        outcol = vec4(color_val.rgb, alpha) * diffuse + S;
}
