precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_ColorTexture; // R, G, B, UNUSED (specular)
uniform sampler2D u_GeometryTexture; // NX_MV, NY_MV, NZ_MV, Depth_MVP
uniform sampler2D u_ShadowTexture; // NX_MV, NY_MV, NZ_MV, Depth_MVP

uniform mat4 u_pT_Matrix;

uniform int u_FragWidth;
uniform int u_FragHeight;

uniform vec3 u_Brightness;

varying vec3 v_mvLightPos;

vec2 samplePoint = vec2(gl_FragCoord.x / float(u_FragWidth), gl_FragCoord.y / float(u_FragHeight));

// Reconstruct MV position from MVP position and inverse P matrix.
vec3 mvPos() {
    float MVP_Z = texture2D(u_GeometryTexture, samplePoint).w;
    vec4 mvpPos = vec4(samplePoint * 2.0 - 1.0, MVP_Z, 1.0);
    vec4 mvPos_hom = u_pT_Matrix * mvpPos;
    return mvPos_hom.xyz / mvPos_hom.w;
}

void main() {

    /*vec3 delta = v_mvLightPos - mvPos();
	
	float distsq = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;

    vec3 normal = texture2D(u_GeometryTexture, samplePoint).xyz;
    
    float diffuse = -dot(normal, normalize(v_mvLightPos));
	    
	float value = u_Brightness.r * diffuse / distsq - .3;
	if(value <= 0.0)
        discard;

    gl_FragColor = vec4(value * texture2D(u_ColorTexture, samplePoint).rgb, 1.0);*/
    
    gl_FragColor = vec4(texture2D(u_ShadowTexture, samplePoint).w, texture2D(u_GeometryTexture, samplePoint).w, 0.0,  1.0);
	
}
