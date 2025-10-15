#version 300 es

precision mediump float; // for gles

uniform bool colorPerVertex;
uniform vec3 color;
uniform float alpha;

in vec3 v_color; // from vertex shader

layout(location = 0) out vec4 fragColor; // output

void main()
{
    if(colorPerVertex){
        fragColor = vec4(v_color, alpha);
    } else {
        fragColor = vec4(color, alpha);
    }
}
