const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
in vec3 FragPos;
uniform sampler2D texture1;
uniform vec3 viewPos;
uniform bool isPlayer;
uniform bool isMoving; // متغير جديد لمعرفة حالة الحركة

void main() {
    vec4 texColor = texture(texture1, TexCoord);
    
    if(isPlayer) {
        if(isMoving) 
            texColor = vec4(0.0, 0.8, 0.2, 1.0); // أخضر وقت المشي
        else 
            texColor = vec4(0.0, 0.4, 0.9, 1.0); // أزرق وقت الوقوف
    }

    float distance = length(viewPos - FragPos);
    float fogDensity = 0.25;
    float fogFactor = exp(-fogDensity * distance);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    vec3 lightColor = texColor.rgb * (1.5 / (1.0 + 0.1 * distance));
    vec3 fogColor = vec3(0.05, 0.05, 0.05);
    
    vec3 finalColor = mix(fogColor, lightColor, fogFactor);
    FragColor = vec4(finalColor, 1.0);
}
)glsl";