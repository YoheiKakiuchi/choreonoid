#version 300 es
precision mediump float;

// 頂点シェーダーから受け取るテクスチャ座標
in vec2 v_texCoord;

// 出力変数
layout(location = 0) out vec4 fragColor;

// Uniform変数
uniform sampler2D textTexture; // テクスチャ（おそらく文字のグリフなど）
uniform vec3 textColor;      // 文字の色

void main()
{
    // テクスチャのRチャンネルをアルファ値として使用
    float alpha = texture(textTexture, v_texCoord).r;
    
    // 文字色にテクスチャからサンプリングしたアルファ値を乗算して最終的な色を決定
    fragColor = vec4(textColor, 1.0) * vec4(1.0, 1.0, 1.0, alpha);
}
