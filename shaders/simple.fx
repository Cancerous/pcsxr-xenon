sampler TextureSampler : register(s0) = sampler_state
{
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};
		
struct VS_INPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};										
										
struct VS_OUT									
{												
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;					
};	

struct PS_IN
{
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};										
										
VS_OUT VS( VS_INPUT In )						
{												
	VS_OUT Out;									
	Out.Position = In.Position;
	Out.TexCoord = In.TexCoord;
	Out.Color = In.Color;
	return Out;
}		

float4 PS( PS_IN In ) : COLOR0
{
	return tex2D( TextureSampler, In.TexCoord );
};