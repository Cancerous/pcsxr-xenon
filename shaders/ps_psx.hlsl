sampler tex0;

struct _IN
{
	float2 uv: TEXCOORD0;
	float4 col : COLOR;
};

float4 main(_IN data): COLOR {
    //return data.col + tex2D(tex0,data.uv);
	#if 0
	return float4(0.7f,0.7f,0.7f,0.7f);
	#else
	float4 Color;
    Color = tex2D( tex0, data.uv) + data.col;
	
	//Color.a = 0.8f;
/*	
	float r = Color.b;
	float g = Color.g;
	float b = Color.r;
	float a = Color.a;
	
	Color.b = b;
	Color.g = g;
	Color.r = r;
	Color.a = a;
*/	
	return  Color;
	
	//return data.col;
	#endif
}
