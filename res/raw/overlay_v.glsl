
attribute vec4 a_Position;
attribute vec2 a_TexCoordinate;
varying vec2 v_TexCoordinate;	

uniform vec2 u_displacement;
uniform vec2 u_scale;

void main() {
    vec2 pos = u_scale * a_Position.xy + u_displacement;
	gl_Position = vec4(pos, 0.0, 1.0);
	v_TexCoordinate = (a_Position.xy + 1.0) / 2.0;
}
