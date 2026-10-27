#include "Lighting.hlsli"

cbuffer Uniforms : register(b0, space1){
    float4x4 mvp;
    float4x4 model;
};

cbuffer LightUniforms : register(b1, space1){
    Light light;
}

cbuffer MateriealUniforms : register(b2, space1)
{
    Material material;
}

struct Input{
    float3 position : TEXCOORD0;
    float2 texcoord : TEXCOORD1;
    float3 normal : TEXCOORD2;
};

struct Output{
    float4 position : SV_Position;
    float2 texcoord : TEXCOORD0;
    float3 diffuse : TEXCOORD1;
    float3 specular : TEXCOORD2;
};

Output main(Input input){
    Output output;
    output.texcoord = (input.texcoord * material.tiling) + material.offset;
    output.position = mul(mvp, float4(input.position, 1.0f));
    
    float3 worldPosition = mul(model, float4(input.position, 1.0f)).xyz;
    
    //Diffuse
    float3 N = normalize(mul((float3x3)model, input.normal));
    
    float3 L;
    if (light.type == LIGHT_DIRECTIONAL){
        L = normalize(-light.lightDir);
    }
    else if (light.type == LIGHT_POINT){
        L = normalize(light.position - worldPosition);     
    }
    
    float NdotL = saturate(dot(N, L));
    
    float3 diffuse = light.lightColor * light.lightIntensity * NdotL; 
    output.diffuse = light.ambient + diffuse;
    
    //Specular
    output.specular = 0;
    if (NdotL > 0)
    {
        float3 R = reflect(-L, N);
        int materialShine = 16;

    }
    
    return output;
}