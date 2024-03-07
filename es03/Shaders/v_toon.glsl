// Vertex shader: Toon shading
// ================
#version 450 core

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform vec3 lightPosition;

// Values that stay constant for the whole mesh.
uniform mat4 P;
uniform mat4 V;
uniform mat4 M; // position*rotation*scaling

out vec3 E;
out vec3 N;
out vec3 L;

void main() {

    gl_Position = P * V * M * vec4(aPos, 1.0);
    
    vec4 eyePosition = V * M * vec4(aPos, 1.0);
	vec4 eyeLightPos = V * vec4(lightPosition, 1.0);

    N = normalize(transpose(inverse(mat3(V * M))) * aNormal);
	L = normalize((eyeLightPos - eyePosition).xyz);
	E = normalize(-eyePosition.xyz);
}

/*
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

struct PointLight {
 	vec3 position;
 	vec3 color;
	float power;
};

uniform PointLight light;
uniform mat4 P, V, M;

out vec3 E, N, L;
	
void main() {
	gl_Position = P * V * M * vec4(aPos, 1.0);
	
	vec4 eyePosition = V * M * vec4(aPos, 1.0);
	vec4 eyeLightPos = V * vec4(light.position, 1.0);

	E = normalize(-eyePosition.xyz);
	N = normalize(transpose(inverse(mat3(V * M))) * aNormal);
	L = normalize((eyeLightPos - eyePosition).xyz);
}
*/