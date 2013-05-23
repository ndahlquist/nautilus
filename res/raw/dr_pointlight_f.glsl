precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_Texture;    // The input texture.

varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec3 v_Normal;         	// Interpolated normal for this fragment.
varying vec2 v_TexCoordinate;   // Interpolated texture coordinate per fragment.

uniform vec3 u_LightPos;

void main() {

    vec3 ob_pos = texture2D(u_Texture, gl_FragCoord.xy / 800.0).rgb;
	
	
	
	//vec3 delta = u_LightPos - pos;
	
	vec3 LightPos = vec3(.4, 0.0, 1.0);
	
	//float distsq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
	
	float distsq = distance(ob_pos, LightPos);

    float val = .4 / distsq;

	gl_FragColor = vec4(val, val, val, 1.0);
}
