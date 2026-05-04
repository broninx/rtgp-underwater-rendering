#version 410 core

#define CAUSTIC_SCALE 10.0
#define CAUSTIC_SPEED 0.02
#define AMBIENT_STR 0.15

layout(location = 0) out vec4 FragColor;

in vec2 Tex;
in vec3 vNormal;
in vec4 viewPosition;
in vec3 worldPos;

uniform sampler2D gTerrainTexture;
uniform sampler2D gTexCaustic;

uniform vec3 gLightDir;
uniform float densityFog;
uniform vec3 topColor;
uniform vec3 botColor;

uniform float gMinHeight;
uniform float gMaxHeight;
uniform float dayPhase;
uniform float uTime;

// ambient, diffusive and specular components (passed from the application)
uniform vec3 specularColor;
// weight of the components
// in this case, we can pass separate values from the main application even if Ka+Kd+Ks>1. In more "realistic" situations, I have to set this sum = 1, or at least Kd+Ks = 1, by passing Kd as uniform, and then setting Ks = 1.0-Kd
uniform float Ka;
uniform float Kd;
uniform float Ks;

// shininess coefficients (passed from the application)
uniform float shininess;


vec3 BlinnPhong_ML(vec3 ambientColor, vec3 diffuseColor) // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // ambient component can be calculated at the beginning
    vec3 color = Ka*ambientColor;

    // normalization of the per-fragment normal
    vec3 N = normalize(vNormal);

    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(gLightDir);

    // Lambert coefficient
    float lambertian = max(dot(L,N), 0.0);

    // if the lambert coefficient is positive, then I can calculate the specular component
    if(lambertian > 0.0)
    {
        // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
        vec3 V = normalize( (-viewPosition).xyz );

        // in the Blinn-Phong model we do not use the reflection vector, but the half vector
        vec3 H = normalize(L + V);

        // we use H to calculate the specular component
        float specAngle = max(dot(H, N), 0.0);
        // shininess application to the specular component
        float specular = pow(specAngle, shininess);

        // We add diffusive and specular components to the final color
        // N.B. ): in this implementation, the sum of the components can be different than 1
        color += vec3( Kd * lambertian * diffuseColor +
                        Ks * specular * specularColor);
    }
    return color;
}

void main()
{
    float causticStr = smoothstep(0.0, 1.0, dayPhase);

    // the fog factor is in base at the distance from the obj and the density of the fog
    float dist = length(viewPosition.xyz);
    float fogFactor = 1.0 - exp(-densityFog * dist);

    //the fog color is in base at the direction of the y from the view of the cam
    float t = smoothstep(gMinHeight, gMaxHeight, worldPos.y);

    vec3 fogColor = mix(botColor, topColor, t);

    vec3 TexColor = texture(gTerrainTexture, Tex).rgb;

    vec3 ambientColor = TexColor * AMBIENT_STR;

    vec3 litColor = BlinnPhong_ML(ambientColor, TexColor);

    //Caustic 
    vec2 causticUV = Tex * CAUSTIC_SCALE + vec2(uTime * CAUSTIC_SPEED, uTime * CAUSTIC_SPEED);
    vec3 caustic = texture(gTexCaustic, causticUV).rgb;

    vec3 N = normalize(vNormal);
    float Diffuse = max(dot(N, gLightDir), 0.0);
    caustic = caustic * causticStr;
    // caustics appear only where light hits 
    vec3 causticContrib = caustic * Diffuse;
    vec3 finalColor = litColor + causticContrib;



    finalColor = mix(finalColor, fogColor, fogFactor);

    FragColor = vec4(finalColor, 1.0);
    // FragColor = vec4(fogColor, 1.0);
}