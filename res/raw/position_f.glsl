precision mediump float;

varying vec3 v_Position;		// Interpolated position for this fragment.

void main() {

    // We map the position to improve its resolution when stored as an 8-bit float.
	gl_FragColor = vec4(v_Position.x / 4.0, v_Position.y / 4.0, (v_Position.z+160.0) / 50.0, 1.0);

}
