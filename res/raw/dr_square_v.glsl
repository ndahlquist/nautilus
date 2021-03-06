
uniform mat4 u_MVMatrix;
uniform mat4 u_MVPMatrix;

attribute vec4 a_Position;

// TODO: This should actually be a uniform.
varying vec3 v_mvLightPos;
varying vec3 v_mvDirVector;

void main() {
	gl_Position = a_Position;
	v_mvLightPos = vec3(u_MVMatrix * vec4(0.0, 0.0, 0.0, 1.0));
	v_mvDirVector = normalize(vec3(u_MVMatrix * vec4(0.0, 1.0, 0.0, 0.0)));
}
