precision highp float;
varying vec2 vTex;
uniform vec3 rgb;
uniform sampler2D uTex;

void main(){
    gl_FragColor = vec4(rgb, 1) * texture2D(uTex, vTex).bgra;
}
