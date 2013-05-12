precision mediump float;       	// Set the default precision to medium. We don't need as high of a precision in the fragment shader.

varying vec3 v_Position;		// Interpolated position for this fragment.
varying vec3 v_Normal;         	// Interpolated normal for this fragment.

varying vec2 v_TexCoordinate;   // This will be passed into the fragment shader.
uniform sampler2D Texture; 

void main() {
    
    vec3 unit_normal = v_Normal / length(v_Normal);

    vec3 u_LightPos = vec3(-300.0, -300.0, 10.0);
    vec3 incidentLight = normalize(u_LightPos - v_Position);
    
	float diffuse = dot(unit_normal, incidentLight);
	if(diffuse <= 0.0)
	    diffuse = 0.0;

	//gl_FragColor = vec4(.1 + .9 * diffuse, diffuse, .2*diffuse, 0.0) 
    vec4 diffuseLight = vec4(.6, .6, .6, 0);
    vec4 materialDiffuse = vec4(.6, .6, .6, 0);
    vec4 light = diffuse * diffuseLight * materialDiffuse;
    gl_FragColor = texture2D(Texture, v_TexCoordinate);
}
