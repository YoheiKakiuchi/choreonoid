#version 300 es
precision highp float;

// 頂点属性：位置(xy)とテクスチャ座標(zw)をパックしたもの
layout (location = 0) in vec4 vertex;

// フラグメントシェーダーへ渡すテクスチャ座標
out vec2 v_texCoord;

uniform mat4 MVP;

void main()
{
    // 頂点のxyを位置、zwをテクスチャ座標として使用
    gl_Position = MVP * vec4(vertex.xy, 0.0, 1.0);
    v_texCoord = vertex.zw;
}
