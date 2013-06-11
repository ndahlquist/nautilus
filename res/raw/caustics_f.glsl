precision mediump float;

// uniform vec2 u_Resolution;
uniform sampler2D u_Texture;
uniform float u_Time;

varying vec2 v_TexCoordinate;

void main() {
  // vec2 uv = gl_FragCoord.xy / iResolution.xy;

  /*float val1 = texture2D(u_Texture, (v_TexCoordinate + vec2(float(u_Time) / 100.0, 0.0)) * .3).r;
  float val2 = texture2D(u_Texture, (v_TexCoordinate + vec2(0.0, float(u_Time) / 100.0)) * .3).r;

  // Higher frequency noise
  float val3 = texture2D(u_Texture, v_TexCoordinate + vec2(float(u_Time) / 20.0, 0.0)).r;
  float val4 = texture2D(u_Texture, v_TexCoordinate + vec2(0.0, float(u_Time) / 20.0)).r;

  float val = (val1 * val2) * (.5 + val3 * val4);*/

  vec4 val1 = texture2D(u_Texture, (v_TexCoordinate + vec2(u_Time / 20.0, 0.0)) * .3);
  vec4 val2 = texture2D(u_Texture, (v_TexCoordinate + vec2(0.0, u_Time / 20.0)) * .3);

  // Higher frequency noise
  vec4 val3 = texture2D(u_Texture, v_TexCoordinate + vec2(u_Time / 20.0, 0.0));
  vec4 val4 = texture2D(u_Texture, v_TexCoordinate + vec2(0.0, u_Time / 20.0));

  vec4 val = (val1 * val2) * (.5 + val3 * val4);

  gl_FragColor = val;
}