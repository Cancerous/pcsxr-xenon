struct PS_IN
{
	float2 Tex   : TEXCOORD0;
	float4 Color : COLOR0;
};
sampler s : register(s0);
float4 main( PS_IN In ) : COLOR0
{
	float4 Color = In.Color;
	return Color;
};