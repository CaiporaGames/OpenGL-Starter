#version 330 core
in vec2 vUV;
in vec4 vColor;
uniform sampler2D uTex;
uniform int u_Mode;  // 0 = normal RGBA, 1 = font: alpha = 1 - red
out vec4 FragColor;

void main() {
    vec4 t = texture(uTex, vUV);
    if (u_Mode == 1) 
    {
        // Font atlas: black glyphs on white background (opaque).
        // Use red channel as coverage and invert it.
        float alpha = 1.0 - t.r;
        FragColor = vec4(vColor.rgb, vColor.a * alpha);
    }
    else if (u_Mode == 2) 
    {
        FragColor = vec4(vColor.rgb, vColor.a * t.a); // PNG alpha
    }
    else 
    {
        FragColor = t * vColor;
    }
}
