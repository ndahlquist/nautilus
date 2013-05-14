precision mediump float;
  
varying vec3 v_Position;		// Interpolated position for this fragment.

void main() {
    
    float depth = clamp((v_Position.z - 5.0) / 60.0, 0.0, 1.0);
	gl_FragColor = vec4((1.0 - depth) , 0.0, depth, 1.0);

}
