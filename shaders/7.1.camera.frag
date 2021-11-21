#version 330 core


struct Light {

vec3 Direction;
vec3 ambient;
vec3 diffuse;
vec3 specular;

};

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

// samplers
uniform sampler2D texture1;
uniform Light light;
uniform vec3 CameraPosition;


void main()
{


    vec3 ambientResult = (light.ambient,1) * texture(texture1, TexCoord).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.Direction);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseResult = light.diffuse * diff * texture(texture1, TexCoord).rgb;  

    vec3 viewDir = normalize(CameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 30);
    vec3 specular = light.specular * spec * texture(texture1, TexCoord).rgb;  

    vec3 finalResult = ambientResult * diffuseResult + 0.1 * texture(texture1, TexCoord).rgb + specular;

	FragColor = vec4(finalResult,1);
}