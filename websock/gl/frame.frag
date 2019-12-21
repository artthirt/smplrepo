precision highp float;
varying vec2 vTex;
uniform sampler2D uTex;
uniform sampler2D uUTex;
uniform sampler2D uVTex;

vec3 getRgb(vec3 yuv)
{
    vec3 vec;

    vec.x = yuv.x + 1.402 * (yuv.z - 0.5);
    vec.y = yuv.x - 0.344 * (yuv.y - 0.5) - 0.714 * (yuv.z - 0.5);
    vec.z = yuv.x + 1.772 * (yuv.y - 0.5);
    return vec;
}

void main(){
    float Y = texture2D(uTex, vTex).x;
    float U = texture2D(uUTex, vTex).x;
    float V = texture2D(uVTex, vTex).x;
    vec3 c = getRgb(vec3(Y, U, V));
    gl_FragColor = vec4(c, 1);
}
