#version 300 es
precision mediump float;
uniform vec4 color;
out vec4 color_out;
void main() {
    color_out = vec4(color.xyz * gl_FragCoord.z, color.w);
}