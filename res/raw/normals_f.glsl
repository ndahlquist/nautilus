precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform vec3 u_LightPos;       	// The position of the light in eye space.
uniform sampler2D u_Texture;    // The input texture.
uniform int u_ProgramMode;      // Specifies the current mode of the program.
  
varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec3 v_Normal;         	// Interpolated normal for this fragment.

void main() {

    gl_FragColor = vec4((v_Normal+1.0) / 2.0, 1.0);

}
