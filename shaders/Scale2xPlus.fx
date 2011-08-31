 
string name : NAME = "ScaleTwoX";
float scaling : SCALING = 2.0;
float2 ps : TEXELSIZE;

matrix World					: WORLD;
matrix View						: VIEW;
matrix Projection				: PROJECTION;
matrix Worldview				: WORLDVIEW;			// world * view
matrix ViewProjection			: VIEWPROJECTION;		// view * projection
matrix WorldViewProjection		: WORLDVIEWPROJECTION;		// world * view * projection

string combineTechique : COMBINETECHNIQUE = "ScaleTwoX";

texture SourceTexture			: SOURCETEXTURE;
texture WorkingTexture			: WORKINGTEXTURE;
texture WorkingTexture1			: WORKINGTEXTURE1;

sampler	decal  = sampler_state {
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = NONE;
};


   float2 video_size;
   float2 texture_size;
   float2 output_size;
   float frame_count;


struct out_vertex {
	float4 position : POSITION;
	//float4 color    : COLOR;
	float2 texCoord : TEXCOORD0;
	float4 t1 : TEXCOORD1;
	float4 t2 : TEXCOORD2;
};

 

 
 
 
//
// Vertex Shader
//

out_vertex VS(	float4 position	: POSITION,
	float4 color	: COLOR,
	float2 texCoord : TEXCOORD0)
{
 
	out_vertex OUT;
 
	OUT.position = position;
	//OUT.color = color;

	float2 ps = float2(1.0/texture_size.x, 1.0/texture_size.y);
	float dx = ps.x;
	float dy = ps.y;

	OUT.texCoord = texCoord; // E
	OUT.t1.xy = texCoord + float2(  0,-dy); // B
	OUT.t1.zw = texCoord + float2(-dx,  0); // D
	OUT.t2.xy = texCoord + float2( dx,  0); // F
	OUT.t2.zw = texCoord + float2(  0, dy); // H

	return OUT;



 
}
 

float4 PS (in out_vertex VAR ) : COLOR
{	
   
	float2 fp = frac(VAR.texCoord*texture_size);

	// Reading the texels

	half3 B = tex2D(decal, VAR.t1.xy).xyz;
	half3 D = tex2D(decal, VAR.t1.zw).xyz;
	half3 E = tex2D(decal, VAR.texCoord).xyz;
	half3 F = tex2D(decal, VAR.t2.xy).xyz;
	half3 H = tex2D(decal, VAR.t2.zw).xyz;

	half3 E0 = D == B && B != H && D != F ? D : E;
	half3 E1 = B == F && B != H && D != F ? F : E;
	half3 E2 = D == H && B != H && D != F ? D : E;
	half3 E3 = H == F && B != H && D != F ? F : E;

	// Product interpolation
	return float4((E3*fp.x+E2*(1-fp.x))*fp.y+(E1*fp.x+E0*(1-fp.x))*(1-fp.y),1);

}


//
// Technique
//

technique ScaleTwoX
{
    pass P0
    {
        // shaders		
        VertexShader = compile vs_3_0 VS();
        PixelShader  = compile ps_3_0 PS(); 
    }  
}
