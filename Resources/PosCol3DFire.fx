//global variables
float4x4 gWorldViewProj 					: WorldViewProjection;
Texture2D gDiffuseMap 						: DiffuseMap;

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;
	
	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;
	
	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;
	
	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;
	
	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

//Blendstate with transparency
BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

//Blendstate without transparency
BlendState gBlendStateNoTransparency
{
	BlendEnable[0] = false;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockwise = true;
};

//-----------------------------------SAMPLES-----------------------------
//Point sample
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Border; // or Mirror or Clamp or Border
	AddressV = Clamp; // or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};
//Linear sample
SamplerState samLinear
{
	Filter = MIN_MAG_POINT_MIP_LINEAR;
	AddressU = Border; // or Mirror or Clamp or Border
	AddressV = Clamp; // or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

//Anisotropic sample
SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Border; // or Mirror or Clamp or Border
	AddressV = Clamp; // or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};


//Input/Output Structs
struct VS_INPUT
{
	float3 Position				: POSITION;
	float3 Color				: COLOR;
	float2 TexCoord				: TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position 			: SV_POSITION;
	float3 Color				: COLOR;
	float2 TexCoord				: TEXCOORD;
};

//Vertex Shader
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	//pos transform
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.TexCoord = input.TexCoord;
	output.TexCoord.y = 1 - output.TexCoord.y;
	
	return output;
}
//------------------------------------------PIXEL SHADERS----------------
//Pixel shader Point tech
float4 PSPoint(VS_OUTPUT input) : SV_TARGET
{
   return float4(gDiffuseMap.Sample(samPoint, input.TexCoord));
}
//Pixel shader Linear tech
float4 PSLinear(VS_OUTPUT input) : SV_TARGET
{
   return float4(gDiffuseMap.Sample(samLinear, input.TexCoord));
}
//Pixel shader Anisotropic tech
float4 PSAnisotropic(VS_OUTPUT input) : SV_TARGET
{
   return float4(gDiffuseMap.Sample(samAnisotropic, input.TexCoord));
}
//------------------------------------------TECHNIQUES----------------
//Techniques
technique11 Techniques
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState , 0);
		SetBlendState(gBlendState, float4(0.0f,0.0f,0.0f,0.0f) , 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSPoint()));
	}
	
	pass P1
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState , 0);
		SetBlendState(gBlendState, float4(0.0f,0.0f,0.0f,0.0f) , 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
	
	pass P2
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState , 0);
		SetBlendState(gBlendState, float4(0.0f,0.0f,0.0f,0.0f) , 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSAnisotropic()));
	}
	//without transparency
	pass P3
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState , 0);
		SetBlendState(gBlendStateNoTransparency, float4(0.0f,0.0f,0.0f,0.0f) , 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSPoint()));
	}
	
	pass P4
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState , 0);
		SetBlendState(gBlendStateNoTransparency, float4(0.0f,0.0f,0.0f,0.0f) , 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
	
	pass P5
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState , 0);
		SetBlendState(gBlendStateNoTransparency, float4(0.0f,0.0f,0.0f,0.0f) , 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSAnisotropic()));
	}
}