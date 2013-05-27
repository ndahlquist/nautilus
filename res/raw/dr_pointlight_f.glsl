precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_ColorTexture; // R, G, B, UNUSED (specular)
uniform sampler2D u_GeometryTexture; // NX_MV, NY_MV, NZ_MV, Depth_MVP
uniform sampler2D u_ShadowTexture; // NX_MV, NY_MV, NZ_MV, Depth_MVP

uniform mat4 u_p_inverse;
uniform mat4 u_mv_inverse;

uniform mat4 u_mv_light;
uniform mat4 u_mv_inverse_light;
uniform mat4 u_mvp_light;

uniform int u_FragWidth;
uniform int u_FragHeight;

uniform vec3 u_Brightness;

varying vec3 v_mvLightPos;

vec2 samplePoint = vec2(gl_FragCoord.x / float(u_FragWidth), gl_FragCoord.y / float(u_FragHeight));

// Reconstruct MV position from MVP position and inverse P matrix.
vec3 mvPos() {
    float MVP_Z = texture2D(u_GeometryTexture, samplePoint).w;
    vec4 mvpPos = vec4(samplePoint * 2.0 - 1.0, MVP_Z, 1.0);
    vec4 mvPos_hom = u_p_inverse * mvpPos;
    return mvPos_hom.xyz / mvPos_hom.w;
}

float readShadowMap(vec3 eyeDir) {
    vec4 projectedEyeDir = u_mvp_light * u_mv_inverse * vec4(eyeDir, 1.0);
    projectedEyeDir = projectedEyeDir / projectedEyeDir.w;
    vec2 shadowTexCoord = projectedEyeDir.xy * vec2(.5, .5) + vec2(.5, .5);
    
    //const float bias = .0001;
    float depthVal = texture2D(u_ShadowTexture, shadowTexCoord).w;
    
    //return projectedEyeDir.z;
    
    //if(projectedEyeDir.z * 0.5 + .5 < depthVal)
    //    return 1.0;
    //return 0.0;
    return projectedEyeDir.y;
    //return projectedEyeDir.z * 0.5 + 0.5 < depthVal;
}

void main() {

    vec3 position = mvPos();

    vec3 light = u_mv_inverse_light[3].xyz;
    vec3 lightDir = normalize(light - position);

    float shadow = readShadowMap(normalize(-position)); 

    vec3 normals = texture2D(u_GeometryTexture, samplePoint).xyw;

    gl_FragColor = vec4(normals, 1.0);


///////////////////////////



    // Calculate if the point is in shadow
    /*vec4 cameraCoord = vec4(mvPos(), 1.0);
    vec4 shadowCoord = u_mvp_light * u_mv_inverse * cameraCoord;
    
    shadowCoord.xyz = shadowCoord.xyz * 0.5;
    shadowCoord.w = shadowCoord.x * 0.5 + shadowCoord.y * 0.5 + shadowCoord.z * 0.5 + shadowCoord.w;
    vec4 shadowCoordinateWdivide = shadowCoord / shadowCoord.w;
  
    // Used to lower moiré pattern and self-shadowing
    shadowCoordinateWdivide.z += 0.0005;

    float distanceFromLight = texture2D(u_ShadowTexture, shadowCoordinateWdivide.xy).z;  
  
    float shadow = 1.0;
    if(shadowCoord.w > 0.0)
        shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.5 : 1.0 ;
  
  
    gl_FragColor = shadow * vec4(1, 0, 1, 1);*/
	
}
