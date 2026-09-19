precision mediump float;

varying vec2 v_uv;
uniform float u_time;

void main() {
    vec2 uv = v_uv;

    float r = 0.5 + 0.5 * sin(uv.x * 10.0 + u_time);
    float g = 0.5 + 0.5 * sin(uv.y * 10.0 + u_time * 1.3);
    float b = 0.5 + 0.5 * sin((uv.x + uv.y) * 10.0 + u_time * 0.7);

    gl_FragColor = vec4(r, g, b, 1.0);
}
