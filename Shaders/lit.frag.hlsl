Texture2D<float4> u_texture : register(t0, space2);
SamplerState u_sampler : register(s0, space2);

cbuffer Uniforms : register(b0, space3){
    float4 tint;
};

struct Input{
    float2 texcoord : TEXCOORD0;
    float3 diffuse : TEXCOORD1;
};

struct Output{
    float4 color : SV_Target0;
};

Output main(Input input){
    Output output;
    
    float4 albedo = u_texture.Sample(u_sampler, input.texcoord) *  tint; 
    output.color = float4(albedo.rgb * input.diffuse, albedo.a);
    
    return output;
}
