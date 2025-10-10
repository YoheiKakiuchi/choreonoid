#version 300 es
precision mediump float; // for gles
precision highp sampler2D;

// Unified SolidPoint shader supporting both MSAA and non-MSAA modes
// Uses uniform bool useMsaa to switch between sampler types at runtime

//#define DO_DEPTH_TEST_IN_GEOMETRY_SHADER 1
//#define DO_DOUBLE_DEPTH_CHECK 1

#ifndef DO_DEPTH_TEST_IN_GEOMETRY_SHADER
#if 0
flat in vec3 pointCenter;
flat in float offsetFragCoord_z;
uniform sampler2D depthTexture2D;
uniform sampler2DMS depthTextureMS;
uniform bool useMsaa;
uniform ivec2 depthTextureSize;
uniform bool isReversedDepth;
#else
// 頂点シェーダーから受け取る変数
flat in vec3 v_pointCenter;
flat in float v_offsetFragCoord_z;
uniform sampler2D depthTexture;
#endif
#endif

#ifdef DO_DOUBLE_DEPTH_CHECK
uniform vec2 viewportSize;
#endif

layout(location = 0) out vec4 fragColor;

uniform vec3 color;

void main()
{
#ifndef DO_DEPTH_TEST_IN_GEOMETRY_SHADER
#if 0 //EM
    float MRD = 1.0 / ((1 << 24) - 1);
    vec2 texCoord = (pointCenter.xy + 1.0) / 2.0;
#else
    float MRD = 1.0 / 16777215.0; // (2^24 - 1) // 24ビット深度バッファを想定した最小解像度差
    vec2 texCoord = (v_pointCenter.xy + 1.0) / 2.0; // NDC座標からテクスチャ座標(0.0 ~ 1.0)へ変換
    float depth = texture(depthTexture, texCoord).r;
    
    if(depth < v_offsetFragCoord_z - MRD){
#endif

    float depth;
    if(useMsaa){
        ivec2 pixelCoord = ivec2(texCoord * vec2(depthTextureSize));
        depth = texelFetch(depthTextureMS, pixelCoord, 0).r;
    } else {
        depth = texture(depthTexture2D, texCoord).r;
    }

    bool shouldDiscard;
    if(isReversedDepth){
        // Reversed depth: larger values are closer
        shouldDiscard = (depth > offsetFragCoord_z + MRD);
    } else {
        // Standard depth: smaller values are closer
        shouldDiscard = (depth < offsetFragCoord_z - MRD);
    }

    if(shouldDiscard){
#ifndef DO_DOUBLE_DEPTH_CHECK
        discard;
#else
#if 0 //EM
        float depth2;
        if(useMsaa){
            ivec2 pixelCoord2 = ivec2(gl_FragCoord.xy);
            depth2 = texelFetch(depthTextureMS, pixelCoord2, 0).r;
        } else {
            vec2 texCoord2 = gl_FragCoord.xy / viewportSize;
            depth2 = texture(depthTexture2D, texCoord2).r;
        }
        bool shouldDiscard2;
        if(isReversedDepth){
            shouldDiscard2 = (depth2 > offsetFragCoord_z + MRD);
        } else {
            shouldDiscard2 = (depth2 < offsetFragCoord_z - MRD);
        }
        if(shouldDiscard2){
#else
        vec2 texCoord2 = gl_FragCoord.xy / viewportSize; // gl_FragCoordから現在のフラグメントのテクスチャ座標を計算
        float depth2 = texture(depthTexture, texCoord2).r; // texture2D()を推奨されるtexture()に変更
        if(depth2 < (v_offsetFragCoord_z - MRD)){
#endif
            discard;
        }
#endif
    }
#endif

    fragColor = vec4(color, 1.0);
}
