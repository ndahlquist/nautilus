precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_ColorTexture; // R, G, B, UNUSED (specular)
uniform sampler2D u_GeometryTexture; // NX_MV, NY_MV, NZ_MV, Depth_MVP
uniform sampler2D u_CausticTexture;

uniform mat4 u_p_inverse;
uniform mat4 u_mv_inverse;

uniform int u_FragWidth;
uniform int u_FragHeight;

uniform vec3 u_Color;
uniform float u_Brightness;

varying vec3 v_mvLightPos;

uniform float u_Time;

vec2 samplePoint = vec2(gl_FragCoord.x / float(u_FragWidth), gl_FragCoord.y / float(u_FragHeight));

// Reconstruct MV position from MVP position and inverse P matrix.
vec4 mvPos() {
    float MVP_Z = texture2D(u_GeometryTexture, samplePoint).w;
    vec4 mvpPos = vec4(samplePoint * 2.0 - 1.0, MVP_Z, 1.0);
    return u_p_inverse * mvpPos;
}

void main() {
    vec4 mvPos = mvPos();
    vec3 delta = v_mvLightPos - mvPos.xyz / mvPos.w;
	float distsq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z; 
	float brightness = min(u_Brightness / distsq, 1.5);
	if(brightness <= 0.0)
	    discard;
	
	vec4 Pos = u_mv_inverse * mvPos;
	vec3 caustic = texture2D(u_CausticTexture, Pos.xy / Pos.w / 100.0).rgb + .6;
    //gl_FragColor = vec4(caustic, 1.0);
    gl_FragColor = vec4(caustic * brightness * u_Color * texture2D(u_ColorTexture, samplePoint).rgb, 1.0);
	
}
