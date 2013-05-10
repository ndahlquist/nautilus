
varying vec3 v_Normal;         	// Interpolated normal for this fragment.

void main() {

    gl_FragColor = vec4((v_Normal.x + 1.0) / 2.0, (v_Normal.y + 1.0) / 2.0, (v_Normal.z + 1.0) / 2.0, 1.0);

}
