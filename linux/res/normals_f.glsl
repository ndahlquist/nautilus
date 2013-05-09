
uniform vec3 u_LightPos;       	// The position of the light in eye space.
uniform sampler2D u_Texture;    // The input texture.
uniform int u_ProgramMode;      // Specifies the current mode of the program.
  
varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec3 v_Normal;         	// Interpolated normal for this fragment.
varying vec3 v_Normal_eye;	    // Interpolated normal for this fragment, in eye space.

void main() {

    gl_FragColor = vec4((v_Normal.x + 1.0) / 2.0, (v_Normal.y + 1.0) / 2.0, (v_Normal.z + 1.0) / 2.0, 1.0);

}
