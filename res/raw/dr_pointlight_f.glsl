precision highp float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_Texture;    // The input texture.

varying vec3 v_Position;		// In MVP space
varying vec3 v_LightPos;        // In MV Space

void main() {

	vec3 pos = texture2D(u_Texture, v_Position.xy).rgb;
	
	vec3 delta = v_LightPos - pos;
	
	float distsq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;

    float val = 200.0 / distsq;

	gl_FragColor = vec4(val, val, val, 1.0);
}
