struct _IN
{
  float4 pos: POSITION;
  float2 uv: TEXCOORD0;
  float4 col: COLOR;
};

struct _OUT
{
  float4 pos: POSITION;
  float2 uv: TEXCOORD0;
  float4 col: COLOR;
};

//float4x4 persp: register (c0);// unused

float2 screenSize : register(c1);

_OUT main(_IN In )
{
  _OUT Out;

  //Out.pos = mul(In.pos,persp);
  // convert coordonate
  // coord 0 - 1
#if 0
  float2 pos;
  pos.x = (((In.pos.x)/screenSize.x)*2.f)-1.f;
  pos.y = (((In.pos.y)/screenSize.y)*2.f)-1.f;
  //pos.x = (In.pos.x/screenSize.x);
  //pos.y = (In.pos.y/screenSize.y);
  Out.pos.xy = pos;
  #else
 Out.pos.x = In.pos.x;
  Out.pos.y = -In.pos.y;
  #endif
  Out.pos.zw = In.pos.zw;
  Out.col = In.col;
  Out.uv = In.uv;
  return Out;
}

