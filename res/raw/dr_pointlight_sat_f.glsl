precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_gBuffer; // R, G, B, Depth_MVP

uniform mat4 u_pT_Matrix;

uniform int u_FragWidth;
uniform int u_FragHeight;

uniform vec3 u_Color;
uniform float u_Brightness;

varying vec3 v_mvLightPos;

vec2 samplePoint = vec2(gl_FragCoord.x / float(u_FragWidth), gl_FragCoord.y / float(u_FragHeight));

// Reconstruct MV position from MVP position and inverse P matrix.
vec3 mvPos() {
    float MVP_Z = texture2D(u_gBuffer, samplePoint).w;
    vec4 mvpPos = vec4(samplePoint * 2.0 - 1.0, MVP_Z, 1.0);
    vec4 mvPos_hom = u_pT_Matrix * mvpPos;
    return mvPos_hom.xyz / mvPos_hom.w;
}

void main() {

    vec3 delta = v_mvLightPos - mvPos();
	float distsq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z; 
	vec3 lightColor = u_Color * u_Brightness / distsq - .2;
    gl_FragColor = vec4(lightColor, 1.0);
	
}
