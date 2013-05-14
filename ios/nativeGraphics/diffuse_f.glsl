precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec3 v_Normal;         	// Interpolated normal for this fragment. 

void main() {

    vec3 u_LightPos = vec3(-300.0, -300.0, 10.0);
    vec3 incidentLight = normalize(u_LightPos - v_Position);
    
	float diffuse = dot(v_Normal, incidentLight);
	if(diffuse <= 0.0)
	    diffuse = 0.0;

	gl_FragColor = vec4(.1 + .9 * diffuse, diffuse, .2*diffuse, 0.0);
}
