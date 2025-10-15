#version 300 es
precision mediump float; // for gles
precision highp sampler2D;

//#define DO_DEPTH_TEST_IN_GEOMETRY_SHADER 1
//#define DO_DOUBLE_DEPTH_CHECK 1

#ifndef DO_DEPTH_TEST_IN_GEOMETRY_SHADER
// 頂点シェーダーから受け取る変数
flat in vec3 v_pointCenter;
flat in float v_offsetFragCoord_z;
uniform sampler2D depthTexture;
#endif

#ifdef DO_DOUBLE_DEPTH_CHECK
uniform vec2 viewportSize;
#endif

layout(location = 0) out vec4 fragColor;

uniform vec3 color;

void main()
{
#ifndef DO_DEPTH_TEST_IN_GEOMETRY_SHADER
    float MRD = 1.0 / 16777215.0; // (2^24 - 1) // 24ビット深度バッファを想定した最小解像度差
    vec2 texCoord = (v_pointCenter.xy + 1.0) / 2.0; // NDC座標からテクスチャ座標(0.0 ~ 1.0)へ変換
    float depth = texture(depthTexture, texCoord).r;
    
    if(depth < v_offsetFragCoord_z - MRD){

#ifndef DO_DOUBLE_DEPTH_CHECK
        discard;
#else
        vec2 texCoord2 = gl_FragCoord.xy / viewportSize; // gl_FragCoordから現在のフラグメントのテクスチャ座標を計算
        float depth2 = texture(depthTexture, texCoord2).r; // texture2D()を推奨されるtexture()に変更
        if(depth2 < (v_offsetFragCoord_z - MRD)){
            discard;
        }
#endif
    }
#endif
    
    fragColor = vec4(color, 1.0);
}
