float2 texture_size: register(c0);

struct out_vertex {
	half4 position : POSITION;
	half4 color    : COLOR0;
	half2 texCoord : TEXCOORD0;
	half4 t1 : TEXCOORD1;
	half4 t2 : TEXCOORD2;
	half4 t3 : TEXCOORD3;
	half4 t4 : TEXCOORD4;
	half4 t5 : TEXCOORD5;
	half4 t6 : TEXCOORD6;
};

out_vertex main(		
	half2 position	: POSITION,
	half2 tex      	: TEXCOORD0,
	half4 color		: COLOR0
)
{	 
	half x = 0.5 * (1.0 / texture_size.x);
	half y = 0.5 * (1.0 / texture_size.y);
	half2 dg1 = half2( x, y);
	half2 dg2 = half2(-x, y);
	half2 sd1 = dg1*0.5;
	half2 sd2 = dg2*0.5;
	half2 ddx = half2(x,0.0);
	half2 ddy = half2(0.0,y);

	out_vertex OUT;
	OUT.position.xy = position.xy;
	OUT.position.z = 0.0;
	OUT.position.w = 1.0;
	OUT.color = color;
	OUT.texCoord = tex;
	OUT.t1 = half4(tex-sd1,tex-ddy);
	OUT.t2 = half4(tex-sd2,tex+ddx);
	OUT.t3 = half4(tex+sd1,tex+ddy);
	OUT.t4 = half4(tex+sd2,tex-ddx);
	OUT.t5 = half4(tex-dg1,tex-dg2);
	OUT.t6 = half4(tex+dg1,tex+dg2);

	return OUT; 
}

