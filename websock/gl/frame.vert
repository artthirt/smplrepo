//precision highp float;
attribute vec3 aVec;
attribute vec2 aTex;
uniform mat4 uMvp;
varying vec2 vTex;
void main(){
    gl_Position = uMvp * vec4(aVec, 1);
    vTex = aTex;
}
