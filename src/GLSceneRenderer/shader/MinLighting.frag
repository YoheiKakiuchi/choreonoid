#version 300 es

// OpenGL ES ではフラグメントシェーダーにおける float と int の精度指定が必須です
precision highp float;
precision highp int;

in vec3 normal; // interpolated normal

uniform int numLights;

struct LightInfo {
    vec3 direction;
    vec3 intensity;
    vec3 ambientIntensity;
};

uniform LightInfo lights[2];

uniform vec3 diffuseColor;
uniform float ambientIntensity;

// GLSL ES 3.00では uniform 変数のシェーダー内での初期化はできません。
// CPU(アプリケーション)側から glUniform3f や glUniform1i を使って
// それぞれ vec3(1.0, 1.0, 1.0) や false を渡すようにしてください。
uniform vec3 highlightColor;
uniform bool isHighlightEnabled;

// ※注意: GLES環境のカラーアタッチメントによっては、vec3出力だと問題が起きる場合があります。
// もし描画に失敗する場合は `layout(location = 0) out vec4 FragColor;` のようにし、
// mainの最後で FragColor = vec4(color, 1.0); のようにRGBA出力へ修正してください。
layout(location = 0) out vec3 color;

void main()
{
    vec3 baseColor = diffuseColor;

    // Highlight mode: replace color with highlight color while preserving texture pattern
    if(isHighlightEnabled) {
        float maxComponent = max(max(baseColor.r, baseColor.g), baseColor.b);
        // Map brightness: 0 -> 0.5, 1 -> 1.0 (preserves texture pattern with boosted dark areas)
        baseColor = highlightColor * (0.5 + maxComponent * 0.5);
    }

    color = vec3(0.0, 0.0, 0.0);
    for(int i=0; i < numLights; ++i){
        LightInfo light = lights[i];
        vec3 n;
        if(gl_FrontFacing){
            n = normalize(normal);
        } else {
            n = -normalize(normal);
        }
        color += light.intensity * baseColor * max(dot(light.direction, n), 0.0);
        color += light.ambientIntensity * ambientIntensity * baseColor;
    }
}
