#version 300 es
precision highp float;

layout (location = 0) in vec4 vertexPosition;
layout (location = 1) in vec3 vertexNormal;

out vec3 normal; // フラグメントシェーダーへ渡す変数

uniform mat4 MVP;
uniform mat3 normalMatrix;

void main()
{
    normal = normalMatrix * vertexNormal; // 変換した法線をフラグメントシェーダーへ渡す
    gl_Position = MVP * vertexPosition; // 頂点位置を変換
}
