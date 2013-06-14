precision mediump float;

// uniform vec2 u_Resolution;
uniform sampler2D u_Texture;
uniform float u_Time;

varying vec2 v_TexCoordinate;
varying float depth_MVP;

void main() {

  // Lower frequency texture
  vec3 val1 = texture2D(u_Texture, (v_TexCoordinate + vec2(u_Time / 20.0, 0.0)) * .3).rgb;
  vec3 val2 = texture2D(u_Texture, (v_TexCoordinate + vec2(0.0, u_Time / 20.0)) * .3).rgb;

  // Higher frequency texture
  vec3 val3 = texture2D(u_Texture, v_TexCoordinate + vec2(u_Time / 20.0, 0.0)).rgb;
  vec3 val4 = texture2D(u_Texture, v_TexCoordinate + vec2(0.0, u_Time / 20.0)).rgb;

  vec3 val = (val1 * val2) * (.5 + val3 * val4);

  gl_FragColor = vec4(val, depth_MVP);
}
