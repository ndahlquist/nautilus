
uniform mat4 u_MVMatrix;
uniform mat4 u_MVPMatrix;

attribute vec4 a_Position;

// TODO: This should actually be a uniform.
varying vec3 v_mvLightPos;

void main() {
	gl_Position = u_MVPMatrix * a_Position;
	v_mvLightPos = vec3(u_MVMatrix * vec4(0.0, 0.0, 0.0, 1.0));
}
