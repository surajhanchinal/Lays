#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 1) in vec3 aColor;
layout (location = 3) in vec3 Indices;
layout (location = 4) in vec3 Weights;
out vec3 FragPos;
out vec3 Normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 jointMatrices[29];
void main()
{
    vec4 totalPos = vec4(0,0,0,0);
    for(int i=0;i<3;i++){
        totalPos = totalPos + Weights[i]*jointMatrices[int(Indices[i])]*vec4(aPos, 1.0);
    }
    //totalPos = jointMatrices[int(Indices[0])]*vec4(aPos,1);
    FragPos = vec3(model * totalPos);
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    gl_Position = projection * view * vec4(FragPos, 1.0);
}