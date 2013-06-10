precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_ColorTexture; // R, G, B, UNUSED (specular)

uniform int u_FragWidth;
uniform int u_FragHeight;

uniform float u_Time;

vec2 samplePoint = vec2(gl_FragCoord.x / 64.0, gl_FragCoord.y / 64.0); // TODO

void main() {
    // Lower frequency noise
    vec3 val1 = texture2D(u_ColorTexture, (samplePoint + vec2(u_Time / 2.0, 0.0)) * .3).rgb;
    vec3 val2 = texture2D(u_ColorTexture, (samplePoint+ vec2(0.0, u_Time / 2.0)) * .3).rgb;

    // Higher frequency noise
    vec3 val3 = texture2D(u_ColorTexture, samplePoint + vec2(u_Time / 10.0, 0.0)).rgb;
    vec3 val4 = texture2D(u_ColorTexture, samplePoint + vec2(0.0, u_Time / 10.0)).rgb;
    
    gl_FragColor = vec4((val1 * val2) * (.5 + val3 * val4), 1.0);
}
