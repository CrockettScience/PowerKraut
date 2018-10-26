#version 450

layout(location = 0) out vec4 out_Color;

void main() {
  out_Color = vec4(sin(gl_FragCoord.x / 960), cos(gl_FragCoord.y / 540), 0.4, 1.0);
}