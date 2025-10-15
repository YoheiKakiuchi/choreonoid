#version 300 es
precision highp float; // for gles

layout (location = 0) in vec3 vertexPosition;

// フラグメントシェーダーへ渡す変数
// flat修飾子は、補間を行わないことを示す
flat out vec3 v_pointCenter;       // NDC座標での点の中心位置
flat out float v_offsetFragCoord_z; // オフセットを加えた深度値

// Uniform variables
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform float pointSize;
uniform float depthOffset;

void main()
{
    vec4 p_view = modelViewMatrix * vec4(vertexPosition, 1.0); // モデルビュー変換
    vec4 finalPosition = projectionMatrix * p_view; // 最終的なクリップ空間座標
    vec4 offsetPosition = projectionMatrix * (p_view + vec4(0, 0, depthOffset, 0.0)); // オフセットを加えたクリップ空間座標

    // set gl variables
    gl_Position = finalPosition;
    gl_PointSize = pointSize;
    
    // フラグメントシェーダーに渡す値を計算
    // クリップ空間座標(.wで除算)からNDC（正規化デバイス座標, -1.0 ~ 1.0）を求める
    v_pointCenter = finalPosition.xyz / finalPosition.w;
    v_offsetFragCoord_z = offsetPosition.z / offsetPosition.w;
}
