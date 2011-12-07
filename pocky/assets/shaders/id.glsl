precision highp float;
uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform vec3 id;
#ifdef _VERTEX_
attribute vec3 in_Position;
attribute vec3 in_TexCoord;

void main(void) {
    gl_Position = projMatrix * modelviewMatrix * vec4(in_Position,1.0);
}
#endif

#ifdef _FRAGMENT_

void main() {
    gl_FragColor = vec4(id.x, id.y, id.z, 1.0);
}
#endif