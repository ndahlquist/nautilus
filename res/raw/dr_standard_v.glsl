
uniform mat4 u_MVMatrix;
uniform mat4 u_MVPMatrix;

attribute vec4 a_Position;

// TODO: This should actually be a uniform.
varying vec3 v_mvLightPos;
varying vec3 v_mvDirVector;

void main() {
    vec4 pos = u_MVPMatrix * a_Position;
    pos.z = 0.0; // Don't clip or depth test
	gl_Position = pos;
	v_mvLightPos = vec3(u_MVMatrix * vec4(0.0, 0.0, 0.0, 1.0));
	v_mvDirVector = normalize(vec3(u_MVMatrix * vec4(0.0, 1.0, 0.0, 0.0)));
}
