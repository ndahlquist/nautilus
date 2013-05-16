precision mediump float;

varying vec3 v_Position;		// Interpolated position for this fragment.

void main() {

	gl_FragColor = vec4(v_Position.x / 10.0, v_Position.y / 10.0, (v_Position.z+150.0) / 100.0, 1.0);

}
