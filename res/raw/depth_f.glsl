precision mediump float;
  
varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec4 v_mvp_Position;

void main() {
    
    float depth = (v_mvp_Position.z / v_mvp_Position.w) / 2.0 + .5;
	gl_FragColor = vec4((1.0 - depth), 0.0, depth, 1.0);

}
