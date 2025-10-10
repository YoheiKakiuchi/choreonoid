#version 300 es
// doubleはGLESに存在しないため、USE_DOUBLE_PRECISION_IN_WIREFRAME_RENDERINGは無効化
precision mediump float;
precision highp int;
precision highp sampler2DShadow; // Shadow Samplerには高い精度を推奨

#define MAX_NUM_LIGHTS 20
#define MAX_NUM_SHADOWS 2
#define USE_BLINN_PHONG_MODEL 1

#if 0
// 頂点シェーダーからの入力
in vec3 v_position;
in vec3 v_normal;
in vec2 v_texCoord;
in vec3 v_colorV;
in vec4 v_shadowCoords[MAX_NUM_SHADOWS];

// ワイヤーフレーム用データ
flat in int v_edgeSituation;
noperspective in vec3 v_edgeDistance;
flat in vec2 v_vertexA;
flat in vec2 v_directionA;
flat in vec2 v_vertexB;
flat in vec2 v_directionB;
#endif

// Uniforms
uniform vec3 diffuseColor;
uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform vec3 emissionColor;
uniform float specularExponent;
uniform float alpha;

uniform int numLights;

struct LightInfo {
    vec4 position;
    vec3 intensity;
    vec3 ambientIntensity;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    float cutoffAngle;
    float beamWidth;
    float cutoffExponent;
    vec3 direction;
};
uniform LightInfo lights[MAX_NUM_LIGHTS];

uniform bool isTextureEnabled;
uniform sampler2D colorTexture;
uniform bool isVertexColorEnabled;
uniform vec3 fogColor;
uniform float maxFogDist;
uniform float minFogDist;
uniform bool isFogEnabled; // 宣言時の初期化は不可

uniform bool isWireframeEnabled;
uniform vec4 wireframeColor;
uniform float wireframeWidth;

uniform int numShadows;
struct ShadowInfo {
    int lightIndex;
    sampler2DShadow shadowMap;
};
uniform ShadowInfo shadows[MAX_NUM_SHADOWS];
uniform bool isShadowAntiAliasingEnabled;

layout(location = 0) out vec4 color4;

// グローバル変数
vec3 reflectionElements[MAX_NUM_LIGHTS];

void main()
{
  color4 = vec4(diffuseColor, 1.0);
}
