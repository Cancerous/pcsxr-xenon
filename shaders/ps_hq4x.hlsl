sampler	decal: register(s0) = sampler_state {
	MinFilter = POINT;
	MagFilter = POINT;
	MipFilter = NONE;
	AddressU  = Clamp;
	AddressV  = Clamp;
} ;

const static half3 dt = half3(1.0, 1.0, 1.0);
const static half mx = 1.0;      // start smoothing wt.
const static half k = -1.10;      // wt. decrease factor
const static half max_w = 0.75;    // max filter weigth
const static half min_w = 0.03;    // min filter weigth
const static half lum_add = 0.33;  // effects smoothing

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

half4 main (in out_vertex VAR) : COLOR0
{	

	half3 c  = tex2D(decal, VAR.texCoord).xyz;
	half3 i1 = tex2D(decal, VAR.t1.xy).xyz; 
	half3 i2 = tex2D(decal, VAR.t2.xy).xyz; 
	half3 i3 = tex2D(decal, VAR.t3.xy).xyz; 
	half3 i4 = tex2D(decal, VAR.t4.xy).xyz; 
	half3 o1 = tex2D(decal, VAR.t5.xy).xyz; 
	half3 o3 = tex2D(decal, VAR.t6.xy).xyz; 
	half3 o2 = tex2D(decal, VAR.t5.zw).xyz;
	half3 o4 = tex2D(decal, VAR.t6.zw).xyz;
	half3 s1 = tex2D(decal, VAR.t1.zw).xyz; 
	half3 s2 = tex2D(decal, VAR.t2.zw).xyz; 
	half3 s3 = tex2D(decal, VAR.t3.zw).xyz; 
	half3 s4 = tex2D(decal, VAR.t4.zw).xyz;  
/*
	half ko1=dot(abs(o1-c),dt);
	half ko2=dot(abs(o2-c),dt);
	half ko3=dot(abs(o3-c),dt);
	half ko4=dot(abs(o4-c),dt);

	half k1=min(dot(abs(i1-i3),dt),max(ko1,ko3));
	half k2=min(dot(abs(i2-i4),dt),max(ko2,ko4));

	half w1 = k2; if(ko3<ko1) w1*=ko3/ko1;
	half w2 = k1; if(ko4<ko2) w2*=ko4/ko2;
	half w3 = k2; if(ko1<ko3) w3*=ko1/ko3;
	half w4 = k1; if(ko2<ko4) w4*=ko2/ko4;

	c=(w1*o1+w2*o2+w3*o3+w4*o4+0.001*c)/(w1+w2+w3+w4+0.001);
	w1 = k*dot(abs(i1-c)+abs(i3-c),dt)/(0.125*dot(i1+i3,dt)+lum_add);
	w2 = k*dot(abs(i2-c)+abs(i4-c),dt)/(0.125*dot(i2+i4,dt)+lum_add);
	w3 = k*dot(abs(s1-c)+abs(s3-c),dt)/(0.125*dot(s1+s3,dt)+lum_add);
	w4 = k*dot(abs(s2-c)+abs(s4-c),dt)/(0.125*dot(s2+s4,dt)+lum_add);

	w1 = clamp(w1+mx,min_w,max_w); 
	w2 = clamp(w2+mx,min_w,max_w);
	w3 = clamp(w3+mx,min_w,max_w); 
	w4 = clamp(w4+mx,min_w,max_w);

	half3 p1 = (w1*(i1+i3)+w2*(i2+i4)+w3*(s1+s3)+w4*(s2+s4)+c);
	half3 p2 = (2.0*(w1+w2+w3+w4)+1.0);
	//return half4((w1*(i1+i3)+w2*(i2+i4)+w3*(s1+s3)+w4*(s2+s4)+c)/(2.0*(w1+w2+w3+w4)+1.0), 1.0);
*/
	float4 ret = float4(c,1.0f);
	return ret;
	//return tex2D(decal, VAR.texCoord);
}