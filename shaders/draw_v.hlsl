									
struct VS_IN	
{												
	float2 ObjPos   : POSITION;				  // Object space position 
	float2 Tex	  : TEXCOORD0;				
	float4 Color    : COLOR0;					  // Vertex color        
};											
										
struct VS_OUT									
{												
	float4 ObjPos   : POSITION;				  // Projected space position 
	float2 Tex	  : TEXCOORD0;				
	float4 Color    : COLOR0;					
};											
										
VS_OUT main( VS_IN In )						
{												
	VS_OUT Out;									
	Out.ObjPos.xy = In.ObjPos.xy;						
	Out.ObjPos.z = 0.0;							
	Out.ObjPos.w = 1.0;							
	Out.Tex = In.Tex;							
	Out.Color = In.Color;						  // Projected space and 
	return Out;									  // Transfer color
}		