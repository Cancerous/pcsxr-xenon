/*
   Author: Themaister
   License: Public domain
*/

 

// The name of this effect
string name : NAME = "StockAspect";
float scaling : SCALING = 2.0;

matrix World				: WORLD;
matrix View					: VIEW;
matrix Projection			: PROJECTION;
matrix Worldview			: WORLDVIEW;			// world * view
matrix ViewProjection			: VIEWPROJECTION;		// view * projection
matrix WorldViewProjection		: WORLDVIEWPROJECTION;		// world * view * projection


string combineTechique : COMBINETECHNIQUE = "StockAspect";


texture SourceTexture			: SOURCETEXTURE;
texture WorkingTexture			: WORKINGTEXTURE;
texture WorkingTexture1			: WORKINGTEXTURE1;

 
sampler	s0 = sampler_state {
	Texture	  = (SourceTexture);
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = NONE; 
};

struct input
{
   float2 video_size;
   float2 texture_size;
   float2 output_size;
   float frame_count;
};

 
  
input IN : VIDPARAMS;

//
// Vertex Shader
//


const float2 src0 = float2(0.6, 0.7);
const float2 src1 = float2(0.9, 0.9);
const float2 src2 = float2(-0.6, 0.3);
const float2 src3 = float2(0.1, 0.4);
const float2 src4 = float2(0.1, 0.4);
const float2 src5 = float2(0.5, 0.5);
const float2 src6 = float2(-1.0, 1.0);


float apply_wave(float2 pos, float2 src, float cnt)
{
   float2 diff = pos - src;
   float dist = 300.0 * sqrt(dot(diff, diff));
   dist -= 0.15 * cnt;
   return sin(dist);
}

void VS(    float4 position : POSITION,
   out float4 oPosition : POSITION,    
   float4 color : COLOR,
   out float4 oColor : COLOR,
   float2 tex : TEXCOORD,
   out float2 oTex : TEXCOORD)
{	 
    oPosition = position;
    oColor = color; 
	oTex = tex;		 
}




float4 PS (float2 tex : TEXCOORD) : COLOR
{  
   float4 output = tex2D(s0, tex);
   float2 scale = tex * IN.texture_size / IN.video_size;

   float cnt = IN.frame_count;
   float res = apply_wave(scale, src0, cnt);
   res += apply_wave(scale, src1, cnt);
   res += apply_wave(scale, src2, cnt);
   res += apply_wave(scale, src3, cnt);
   res += apply_wave(scale, src4, cnt);
   res += apply_wave(scale, src5, cnt);
   res += apply_wave(scale, src6, cnt);

   return output * (0.9 + 0.012 * res);
}


//
// Technique
//

technique StockAspect
{
    pass P0
    {
        // shaders
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS(); 
    }  
}
