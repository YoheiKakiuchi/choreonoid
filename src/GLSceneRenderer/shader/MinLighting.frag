#version 300 es
precision mediump float;
precision highp int;

in vec3 v_normal; // 頂点シェーダーから受け取る（補間された）法線ベクトル

// 出力変数
layout(location = 0) out vec4 fragColor;

// ループで使うため、配列サイズは定数で定義するのが望ましい
#define MAX_NUM_LIGHTS 2

struct LightInfo {
    vec3 direction;
    vec3 intensity;
    vec3 ambientIntensity;
};

uniform int numLights;
uniform LightInfo lights[MAX_NUM_LIGHTS];

uniform vec3 diffuseColor;
uniform vec3 ambientColor;

void main()
{
    vec3 color = vec3(0.0);
    
    // 頂点シェーダーから渡された法線は補間されているため、
    // 正確なライティングのために必ず正規化する
    vec3 n = normalize(v_normal);

    // ポリゴンの裏面を描画している場合は、法線を反転させる
    if(!gl_FrontFacing){
        n = -n;
    }

    // uniformで受け取ったライトの数だけループ
    for(int i = 0; i < MAX_NUM_LIGHTS; ++i)
    {
        // 有効なライトの数を超えたらループを抜ける
        if(i >= numLights) {
            break;
        }

        // 拡散反射光の計算
        float diff = max(dot(lights[i].direction, n), 0.0);
        color += lights[i].intensity * diffuseColor * diff;
        
        // 環境光の計算
        color += lights[i].ambientIntensity * ambientColor;
    }

    // 計算した色と、アルファ値1.0で出力
    fragColor = vec4(color, 1.0);
}
