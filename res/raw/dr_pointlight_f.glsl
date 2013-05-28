precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_ColorTexture; // R, G, B, UNUSED (specular)
uniform sampler2D u_GeometryTexture; // NX_MV, NY_MV, NZ_MV, Depth_MVP
uniform sampler2D u_ShadowTexture; // NX_MV, NY_MV, NZ_MV, Depth_MVP

uniform mat4 u_p_inverse;
uniform mat4 u_mv_inverse;

uniform mat4 u_mv_light;
uniform mat4 u_mvp_light;

uniform int u_FragWidth;
uniform int u_FragHeight;

uniform vec3 u_Brightness;

varying vec3 v_mvLightPos;

vec2 samplePoint = vec2(gl_FragCoord.x / float(u_FragWidth), gl_FragCoord.y / float(u_FragHeight));

// Reconstruct MV position from MVP position and inverse P matrix.
vec4 mvPos() {
    float MVP_Z = texture2D(u_GeometryTexture, samplePoint).w;
    vec4 mvpPos = vec4(samplePoint * 2.0 - 1.0, MVP_Z, 1.0);
    vec4 mvPos = u_p_inverse * mvpPos;
    return mvPos;
}

void main() {

    vec4 mvPos = mvPos(); // Subject position in MV space.
    vec4 Pos = u_mv_inverse * mvPos; // Subject position in world space.
    
    vec3 lightRay = Pos.xyz / Pos.w; // Light vector in world space.
    vec4 mvplightRay = u_mvp_light * vec4(lightRay, 0.0); // Light vector in light's MVP space.
    mvplightRay.xyz = normalize(mvplightRay.xyz);
    
    vec2 shadowTexCoord = mvplightRay.xy * .5 + .5;
    
    float depthVal = texture2D(u_ShadowTexture, shadowTexCoord).w;
    
    float shadow;
    if(mvplightRay.z * 0.5 + .5 < depthVal)
        shadow = 1.0;
    else
        shadow = 0.1;
    
    vec3 albedo = texture2D(u_ColorTexture, samplePoint).rgb;
    
    gl_FragColor = vec4(shadow * albedo, 1.0);
	
}
