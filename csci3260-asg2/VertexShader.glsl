#version 430

in layout(location = 0) vec3 position;
in layout(location = 1) vec2 vertexUV;
in layout(location = 2) vec3 vertexNormal;


out vec3 FragPos;
out vec2 UV;
out vec3 normal;
out vec4 FragPosLightSpace;
out int mode;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform int shadowMode;

void main()
{
	
	/*mode = shadowMode;
	if (shadowMode==1) {
		gl_Position = lightSpaceMatrix * model * vec4(position, 1.0f);
	}
	else {*/
		gl_Position = projection * view * model * vec4(position, 1.0f);
		FragPos = vec3(model * vec4(position, 1.0f));
		UV = vertexUV;
		normal = mat3(transpose(inverse(model))) * vertexNormal;
	//	FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
	//}
}