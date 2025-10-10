#version 300 es
precision highp float;

// 入力頂点属性
layout (location = 0) in vec4 vertexPosition;
layout (location = 1) in vec3 vertexNormal;

// Uniform変数
uniform mat4 MVP;
uniform mat3 normalMatrix;

void main()
{
    // 法線をビュー座標系に変換し、正規化
    vec3 normal = normalize(normalMatrix * vertexNormal);
    
    // 変換後の座標を、法線方向に少しずらす
    gl_Position = MVP * vertexPosition + vec4(normal * 0.01, 0.0);
}