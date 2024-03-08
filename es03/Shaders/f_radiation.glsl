#version 450 core

in vec3 Color;
out vec4 FragColor;

void main() {
    // Calcoliamo un fattore di emissione basato sul colore e sulla luminosità
    vec3 emission = Color * 0.5 + vec3(0.0, 0.2, 0.0); // Adattare il colore e l'intensità del glow

    // Mixiamo il colore originale con l'emissione per ottenere l'effetto di glow
    vec3 finalColor = mix(Color, emission, 0.5); // Adattare la quantità di glow necessaria

    FragColor = vec4(finalColor, 1.0);
}

/*
out vec4 FragColor;

in vec3 E; 
in vec3 N; 
in vec3 L;
uniform vec4 Color;

uniform float time;
uniform float freq;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main() {
    // Compute vectors E,L,R in VCS
    vec3 E = normalize(-eyePosition.xyz);
    vec3 L = normalize((eyeLightPos - eyePosition).xyz);
    vec3 R = reflect(-L, N);  

    // ambient
    vec3 ambient = light.power * material.ambient;
    
    // diffuse 
    float diff = max(dot(L,N), 0.0);
    vec3 diffuse = light.power * light.color * diff * material.diffuse;

    // specular
    float spec = pow(max(dot(E, R), 0.0), material.shininess);
    vec3 specular =  light.power * light.color * spec * material.specular;  

    // Calculate pulsing effect for ambient, diffuse, and specular
    float pulse = abs(sin(freq * time));

    // Apply pulsing effect to ambient, diffuse, and specular
    ambient *= pulse;
    diffuse *= pulse;
    specular *= pulse;

    Color = ambient + diffuse + specular;
}*/