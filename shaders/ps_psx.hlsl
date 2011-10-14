sampler tex0;

struct _IN
{
    float2 uv: TEXCOORD0;
    float2 uv1: TEXCOORD1;
    float4 col : COLOR0;
};

float4 psZ(_IN data): COLOR {
    float4 Color;

    Color = data.col/data.uv1.x;

    return  Color;
}

float4 psG(_IN data): COLOR {
    float4 Color;

    Color = tex2D( tex0, data.uv.xy) * data.col;

    return  Color;
}

float4 psF(_IN data): COLOR {
    float4 Color;

    Color = tex2D( tex0, data.uv.xy);// + data.col;
	
    return  Color;
}

float4 psC(_IN data): COLOR {
    float4 Color;

    Color = data.col;
	
    return  Color;
}

float4 main(_IN data): COLOR {
    return psF(data);
}