#version 300 es

precision mediump float; // for gles
uniform vec3 color;
//in vec3 v_color; // from vertex shader
layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(color, 1.0);
    //fragColor = vec4(v_color, 1.0);
}
