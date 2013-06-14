precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

varying float depth_MVP;

void main() {
	gl_FragColor = vec4(1.0, 1.0, 1.0, depth_MVP);
}
