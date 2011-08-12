struct PS_IN
{
	float2 Tex   : TEXCOORD0;
	float4 Color : COLOR0;
};

sampler s : register(s0) = sampler_state {
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = NONE;
	AddressU  = Clamp;
	AddressV  = Clamp;
} ;

float4 main( PS_IN In ) : COLOR0
{
	return tex2D( s, In.Tex );
};