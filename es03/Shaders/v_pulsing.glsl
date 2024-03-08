#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Color;

uniform vec3 lightPosition;

uniform float time;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M; // position*rotation*scaling

struct PointLight{
	vec3 position;
	vec3 color;
	float power;
 };
uniform PointLight light;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
uniform Material material;

out vec3 E;
out vec3 N;
out vec3 L;

float a = 0.1;
float freq = 0.001;

void main() {
    // Compute the vertex position in world space
    vec4 worldPosition = M * vec4(aPos, 1.0);
    
	// Position in VCS
	vec4 eyePosition = V * M * vec4(aPos, 1.0);
	// LightPos in VCS
	vec4 eyeLightPos = V * vec4(light.position, 1.0);
	// Compute vectors H,L,N in VCS
	E = normalize(-eyePosition.xyz);
	L = normalize((eyeLightPos - eyePosition).xyz);
	N = normalize(transpose(inverse(mat3(V * M))) * aNormal);
	H = normalize(L + E);
    vec3 R = reflect(-L, N);  	
    // ambient
    vec3 ambient = light.power * material.ambient;	
	// diffuse 
    float diff = max(dot(L,N), 0.0);
    vec3 diffuse = light.power * light.color * diff * material.diffuse;
    // specular
    float spec = pow(max(dot(E, R), 0.0), material.shininess);
    vec3 specular =  light.power * light.color * spec * material.specular; 
    Color = ambient + diffuse + specular;
	
    // Calculate the distance from the vertex to the center of the object
    float distanceToCenter = length(worldPosition.xyz);

    // Calculate the pulsing effect based on the distance to the center
    float pulseIntensity = abs(sin(freq * time + distanceToCenter));

    // Apply the pulsing effect to the vertex position
    vec3 pulsatingPosition = aPos + aNormal * pulseIntensity * 0.1; // Adjust the pulsating effect intensity as needed

    // Transform the pulsating vertex position to clip space
    gl_Position = P * V * M * vec4(pulsatingPosition, 1.0);

    // Other calculations...
}