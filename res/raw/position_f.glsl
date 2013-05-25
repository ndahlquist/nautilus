precision mediump float;

varying vec3 v_Position;
varying vec4 v_MVP_Position;

void main() {
    
    gl_FragColor = vec4(v_MVP_Position.xyz / v_MVP_Position.w, 1.0); // Convert from homogeneous coords

}
