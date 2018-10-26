#version 450

out gl_PerVertex
{
  vec4 gl_Position;
};

void main() {
    vec2 pos[14] = vec2[14](vec2(-0.5, -0.7), vec2(-0.5, 0.7), vec2(-0.4, 0.7), vec2(-0.4, 0.0), vec2(-0.1, -0.35), vec2(-0.4, -0.7), vec2(-0.5, -0.7), 
							vec2(0.1, -0.7), vec2(0.1, 0.7), vec2(0.2, 0.7), vec2(0.5, 0.7), vec2(0.2, 0.0), vec2(0.5, -0.7), vec2(0.2, -0.7));
    gl_Position = vec4( pos[gl_VertexIndex + (7 * gl_InstanceIndex)], 0.0, 1.0 );
}