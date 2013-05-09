precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

//uniform vec3 u_LightPos;       	// The position of the light in eye space.
uniform sampler2D u_Texture;    // The input texture.
  
varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec3 v_Normal;         	// Interpolated normal for this fragment.
varying vec3 v_Normal_eye;	    // Interpolated normal for this fragment, in eye space.
varying vec2 v_TexCoordinate;   // Interpolated texture coordinate per fragment.

void main() {
    
    vec3 u_LightPos = vec3(3.0, 3.0, 3.0);
    
    vec3 incidentLight = normalize(u_LightPos - v_Position);
	float diffuse = dot(v_Normal, incidentLight);
	if(diffuse <= 0.0)
	    diffuse = 0.0;
	else
	    diffuse *= 4.0 / length(u_LightPos - v_Position);

	gl_FragColor = vec4(.1+diffuse, diffuse, .2*diffuse, 1.0);// * texture2D(u_Texture, v_TexCoordinate);
}
