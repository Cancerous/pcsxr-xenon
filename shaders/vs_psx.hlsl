struct _IN
{
    float3 pos: POSITION;
    float2 uv: TEXCOORD0;
    float4 col: COLOR0;
};

struct _OUT
{
    float4 pos: POSITION;
    float2 uv: TEXCOORD0;
    float4 col: COLOR0;
};

float2 screenSize : register(c1);

_OUT main(_IN In )
{
    _OUT Out;

    float2 pos;
    pos.x = (((In.pos.x)/screenSize.x)*2.f)-1.f;
    pos.y = (((In.pos.y)/screenSize.y)*2.f)-1.f;

    Out.pos.xy = pos;
    Out.pos.y = -Out.pos.y;
    Out.pos.z = In.pos.z;
    Out.pos.w = 1.0f;
    Out.col = In.col;
    Out.uv = In.uv;
    return Out;
}

