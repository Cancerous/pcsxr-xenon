struct _IN
{
    float3 pos: POSITION;
    float2 uv0: TEXCOORD0;
    float2 uv1: TEXCOORD1;
    float4 col: COLOR0;
};

struct _OUT
{
  float4 pos: POSITION;
  float2 uv0: TEXCOORD0;
  float2 uv1: TEXCOORD1;
  float4 col: COLOR;
};

float4x4 mwp : register(c0);
float2 screenSize : register(c1);

_OUT main(_IN In )
{
    _OUT Out;

    float4 pos = float4(In.pos,1.0f);
    // apply psx viewport
    Out.pos.x = (((In.pos.x)/screenSize.x)*2.f)-1.f;
    Out.pos.y = (((In.pos.y)/screenSize.y)*2.f)-1.f;
    Out.pos.z = 1-In.pos.z;
    Out.pos.w = 1.0f;

    // reverse ...
    Out.pos.y = -Out.pos.y;

    //Out.pos = mul(mwp, pos);

    Out.col = In.col;
    Out.uv0 = In.uv0;
    Out.uv1 = In.uv1;
    return Out;
}

