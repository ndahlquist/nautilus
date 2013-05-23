precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_Texture;    // The input texture.   		       
uniform mat4 u_MVMatrix;		// A constant representing the combined model/view matrix.

varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec3 v_Normal;         	// Interpolated normal for this fragment.
varying vec2 v_TexCoordinate;   // Interpolated texture coordinate per fragment.

uniform vec3 u_LightPos;
uniform int u_FragWidth;
uniform int u_FragHeight;

void main() {

    vec3 LightPos = vec3(u_MVMatrix * vec4(u_LightPos, 0.0));

    vec3 normpos = texture2D(u_Texture, vec2(gl_FragCoord.x / float(u_FragWidth), gl_FragCoord.y / float(u_FragHeight))).rgb;
    vec3 pos = vec3(normpos.x*10.0, normpos.y*10.0, normpos.z*100.0 - 150.0); // Reverse the mapping function.
	
	vec3 delta = LightPos - pos;
	
	float distsq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;

    float val = 800.0 / distsq;

	gl_FragColor = vec4(val, val, val, 1.0);
}
