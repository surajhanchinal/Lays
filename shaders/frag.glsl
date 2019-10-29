#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;
uniform vec3 color;
uniform vec3 LightPos;
void main()
{
    // ambient
    float ambient = 0.4;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    ambient = ambient + abs(norm.x);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    
    // specular
    //float specularStrength = 0.5;
    //vec3 viewDir = normalize(viewPos - FragPos);
    //vec3 reflectDir = reflect(-lightDir, norm);  
    //float spec = specularStrength*pow(max(dot(viewDir, reflectDir), 0.0), 32);
        
    vec3 result = (ambient + diff) * color;
    FragColor = vec4(result, 1.0);
    //FragColor = vec4(color,1.0); // set all 4 vector values to 1.0
}
