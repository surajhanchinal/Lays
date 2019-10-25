#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
//layout (location = 2) in vec3 Color;
out vec3 Normal;
out vec3 FragPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 out_model;
uniform mat4 projection;
void main()
{
    mat4 new_model = out_model*model;
    vec3 FragPos = vec3(new_model * vec4(aPos, 1.0));
    gl_Position = projection * view *  vec4(FragPos, 1.0);
    Normal = mat3(transpose(inverse(new_model))) * aNormal;  
}