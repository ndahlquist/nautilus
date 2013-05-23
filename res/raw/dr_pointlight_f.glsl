precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_PosTexture;
uniform sampler2D u_AlbTexture;
uniform sampler2D u_NormTexture;

uniform mat4 u_MVMatrix;		// A constant representing the combined model/view matrix.
uniform mat4 u_pT_Matrix;

varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec3 v_Normal;         	// Interpolated normal for this fragment.
varying vec2 v_TexCoordinate;   // Interpolated texture coordinate per fragment.

uniform int u_FragWidth;
uniform int u_FragHeight;

void main() {    

    vec2 samplePoint = vec2(gl_FragCoord.x / float(u_FragWidth), gl_FragCoord.y / float(u_FragHeight));
    
    vec3 LightPos = vec3(u_MVMatrix * vec4(0.0, 0.0, 0.0, 1.0));

    float MVP_Z = texture2D(u_PosTexture, samplePoint).z;
    
    vec4 mvpPos = vec4(samplePoint * 2.0 - 1.0, MVP_Z, 1.0);
    
    vec4 mvPos_hom = u_pT_Matrix * mvpPos;
    
    vec3 mvPos = mvPos_hom.xyz / mvPos_hom.w;
	
	vec3 delta = LightPos - mvPos;
	
	float distsq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;

    vec3 normal = texture2D(u_NormTexture, samplePoint).xyz;
    
    float diffuse = dot(normal, normalize(-LightPos));
	if(diffuse <= 0.0)
	    diffuse = 0.0;

	gl_FragColor = 120.0 * (diffuse + .1) * texture2D(u_AlbTexture, samplePoint) / distsq;
	
}
