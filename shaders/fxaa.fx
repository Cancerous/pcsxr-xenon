#define FXAA_HLSL_3 1

#include "fxaa_v2.h"

#define W	1280.f
#define H	720.f

uniform float2 texels = {
	1.f/W,
	1.f/H
};


sampler Sampler;

//-----------------------------------------------------------------------------
// Vertex Definitions
//-----------------------------------------------------------------------------

struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 TexCoord : TEXCOORD0;
};

struct VS_INPUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT
{
	float4 TexCoord : TEXCOORD0;
};

VS_OUTPUT vs_fxaa(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;
	
	Output.Position = Input.Position;
	//Output.TexCoord = FxaaVertexShader(Input.TexCoord,texels);
	
	float4 posPos;
    posPos.xy = Input.TexCoord;
    posPos.zw = posPos.xy - (texels * (0.5 + FXAA_SUBPIX_SHIFT));

	Output.TexCoord = posPos;
	
	return Output;
}

float4 ps_fxaa(PS_INPUT Input) : COLOR
{
	//return tex2D(Sampler, Input.TexCoord);
	//return float4(0.5,0.3,0.9,0.5);
	
	//return float4(FxaaPixelShader(Input.TexCoord, Sampler, texels).xyz, 0.0f);
	/*
	return float4(
		FxaaPixelShader(
			Input.TexCoord, 
			Sampler, 
			texels
		).xyz, 
		0.0f
	);
	*/
		return float4(
		FxaaPixelShader(
			Input.TexCoord, 
			Sampler, 
			texels
		).xyz, 
		0.0f
	);
}
