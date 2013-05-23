precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_Texture;    // The input texture.   		       
uniform mat4 u_MVMatrix;		// A constant representing the combined model/view matrix.

varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec3 v_Normal;         	// Interpolated normal for this fragment.
varying vec2 v_TexCoordinate;   // Interpolated texture coordinate per fragment.

uniform int u_FragWidth;
uniform int u_FragHeight;

void main() {

    vec3 LightPos = vec3(u_MVMatrix * vec4(0.0, 0.0, 0.0, 1.0));

    vec3 mappos = texture2D(u_Texture, vec2(gl_FragCoord.x / float(u_FragWidth), gl_FragCoord.y / float(u_FragHeight))).rgb;
    vec3 pos = vec3(mappos.x*10.0, mappos.y*10.0 * 5.0, mappos.z*100.0 - 150.0); // Reverse the mapping function. // TODO: 5 multiplier
	
	vec3 delta = LightPos - pos;
	
	float distsq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;

    float val = 20.0 / distsq;

	gl_FragColor = vec4(val, val, val, 1.0);
}
