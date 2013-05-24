precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

varying vec3 v_Normal;         	// Interpolated normal for this fragment.

varying vec3 v_Position;
varying vec4 v_MVP_Position;

void main() {

    float depth = v_MVP_Position.z / v_MVP_Position.w;
    gl_FragColor = vec4(v_Normal, depth);

}
