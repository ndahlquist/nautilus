precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_PosTexture;
uniform sampler2D u_AlbTexture;       
uniform mat4 u_MVMatrix;		// A constant representing the combined model/view matrix.

varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec3 v_Normal;         	// Interpolated normal for this fragment.
varying vec2 v_TexCoordinate;   // Interpolated texture coordinate per fragment.

uniform int u_FragWidth;
uniform int u_FragHeight;

void main() {

    vec2 samplePoint = vec2(gl_FragCoord.x / float(u_FragWidth), gl_FragCoord.y / float(u_FragHeight));

    vec3 LightPos = vec3(u_MVMatrix * vec4(0.0, 0.0, 0.0, 1.0));

    vec3 mappos = texture2D(u_PosTexture, samplePoint).rgb;
    vec3 pos = vec3(mappos.x*4.0, mappos.y*4.0, mappos.z*50.0 - 160.0); // Reverse the mapping function.
	
	vec3 delta = LightPos - pos;
	
	float distsq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;

    float val = 20.0 / distsq;

	gl_FragColor = val * texture2D(u_AlbTexture, samplePoint);
}
