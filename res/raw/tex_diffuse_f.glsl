precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

uniform sampler2D u_Texture;    // The input texture.

varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec3 v_Normal;         	// Interpolated normal for this fragment.
varying vec2 v_TexCoordinate;   // Interpolated texture coordinate per fragment.

void main() {
    
    /*vec3 u_LightPos = vec3(-300.0, -300.0, 10.0);
    vec3 incidentLight = normalize(u_LightPos - v_Position);
    
	float diffuse = dot(v_Normal, incidentLight);
	if(diffuse <= 0.0)
	    diffuse = 0.0;*/
	   
    vec3 tex = texture2D(u_Texture, v_TexCoordinate).xyz;

	//gl_FragColor = vec4(diffuse * tex, 1.0);
	gl_FragColor = vec4(tex, 1.0);
}
