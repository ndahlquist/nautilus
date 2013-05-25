
uniform mat4 u_MVPMatrix;		// A constant representing the combined model/view/projection matrix.      		       
uniform mat4 u_MVMatrix;		// A constant representing the combined model/view matrix.

attribute vec2 a_TexCoordinate; // Per-vertex texture coordinate information we will pass in.
varying vec2 v_TexCoordinate;   // This will be passed into the fragment shader. 		

attribute vec3 a_Normal;
varying vec3 v_Normal;

attribute vec4 a_Position;		// Per-vertex position information we will pass in.
varying float depth_MVP;

void main() {
	v_TexCoordinate = a_TexCoordinate;
	v_Normal = normalize(vec3(u_MVMatrix * vec4(a_Normal, 0.0)));

	// Multiply the vertex by the matrix to get the final point in normalized screen coordinates.
	vec4 v_MVP_Position = u_MVPMatrix * a_Position;
	gl_Position = v_MVP_Position;
	depth_MVP = v_MVP_Position.z / v_MVP_Position.w;
}
