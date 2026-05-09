#version 440

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 v_color;

void main() {
    // 座標をそのまま出力
    gl_Position = vec4(position, 0.0, 1.0);
    // 色をフラグメントシェーダーに渡す
    v_color = color;
}