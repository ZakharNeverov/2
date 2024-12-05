#version 330 core

in vec3 fragPos;
in vec3 normalInterp;
in vec2 texCoordInterp;

uniform sampler2D textureSampler;
uniform bool useTexture; // Флаг использования текстуры
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float alpha; // Альфа-канал для прозрачности

uniform vec3 ambientLight; // Фоновый свет

// Параметры материала
uniform vec3 materialSpecular;   // Спекулярный цвет
uniform vec3 materialDiffuse;    // Диффузный цвет
uniform vec3 materialAmbient;    // Фоновый цвет материала
uniform float materialShininess; // Коэффициент блеска

out vec4 FragColor;

void main()
{
    vec3 color;
    if (useTexture) {
        color = texture(textureSampler, texCoordInterp).rgb;
    } else {
        color = materialDiffuse;
    }

    // Нормализуем нормаль
    vec3 normal = normalize(normalInterp);
    
    // Направление к источнику света
    vec3 lightDir = normalize(lightPos - fragPos);
    
    // Направление к камере
    vec3 viewDir = normalize(viewPos - fragPos);
    
    // Направление отражения света
    vec3 reflectDir = reflect(-lightDir, normal);
    
    // Расчёт диффузного освещения
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * color * lightColor * lightIntensity;
    
    // Расчёт френелевского эффекта (для улучшения спекуляра)
    float fresnel = pow(1.0 - max(dot(viewDir, normal), 0.0), 5.0);

    // Расчёт спекулярного освещения с использованием френеля
    float spec = 0.0;
    if(diff > 0.0){
        spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    }
    vec3 specular = (1.0 - fresnel) * spec * materialSpecular * lightIntensity;
    
    // Расчёт фонового освещения
    vec3 ambient = ambientLight * materialAmbient;
    
    // Итоговый цвет фрагмента
    vec3 finalColor = ambient + diffuse + specular;
    
    // Финальный цвет с альфа-каналом
    FragColor = vec4(finalColor, alpha);
}
