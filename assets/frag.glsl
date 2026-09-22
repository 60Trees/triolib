/**
 * @author 60Trees_ (github.com/60Trees)
 * @version 1.0.0
 */

#extension GL_ARB_gpu_shader_int64 : enable
#if defined(GL_ARB_gpu_shader_int64)
uniform uint64_t u_time;
#else
uniform uint u_time;
#endif

out vec4 FragColor;


uniform ivec2 u_windowsize;

void main() {
    float t = float(u_time) / 1000.0;

    vec2 uv = gl_FragCoord.xy / vec2(u_windowsize);

    FragColor = vec4(
        0.5 + 0.5 * sin(t + uv.x * 3.0),
        0.5 + 0.5 * sin(t + 2.094),
        0.5 + 0.5 * sin(t + 4.189),
        1.0
    );
}
