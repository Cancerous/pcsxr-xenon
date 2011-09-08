sampler tex0;
bool b_mul_tex;

struct _IN
{
    float2 uv: TEXCOORD0;
    float4 col : COLOR;
};

float4 main(_IN data): COLOR {
    float4 Color;
    if( b_mul_tex)
    {
        Color = tex2D( tex0, data.uv) * data.col;
    }
    else{
        Color = tex2D( tex0, data.uv) + data.col;
    }
    return  Color;
}
