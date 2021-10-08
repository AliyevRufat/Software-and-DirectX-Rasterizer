//global variables
float4x4 gWorldViewProj 					: WorldViewProjection;
float4x4 gWorld								: WorldMatrix;
float4x4 gViewInversed						: ViewInverseMatrix;

float gPi = 3.14159265359f;

Texture2D gDiffuseMap 						: DiffuseMap;
Texture2D gNormalMap						: NormalMap;
Texture2D gSpecularMap						: SpecularMap;
Texture2D gGlossinessMap					: GlossinessMap;


//-------------------------------------------------------------------------states---------------------------------------------
DepthStencilState gDepthStencilState
{
	DepthEnable = true;	
	DepthWriteMask = all;
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

RasterizerState gRasterizerStateBackCulling
{
	CullMode = back;
	FrontCounterClockwise = true;
};

RasterizerState gRasterizerStateFrontCulling
{
	CullMode = front;
	FrontCounterClockwise = true;
};

RasterizerState gRasterizerStateNone
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
	float3 WorldPosition		: COLOR;
	float2 TexCoord				: TEXCOORD;
	float3 Normal 				: NORMAL;
	float3 Tangent				: TANGENT;
};

struct VS_OUTPUT
{
	float4 Position 			: SV_POSITION;
	float4 WorldPosition		: COLOR;
	float2 TexCoord				: TEXCOORD;
	float3 Normal 				: NORMAL;
	float3 Tangent				: TANGENT;
};

//Vertex Shader
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	//pos transform
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorld);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorld);
	output.WorldPosition = mul(float4(input.Position, 1.0), gWorld);
	output.TexCoord = input.TexCoord;
	output.TexCoord.y = 1 - output.TexCoord.y;

	return output;
}
//------------------------------------------PIXEL SHADERS----------------

//normal calculation
float3 CalculateNewNormal(float3 tangent, float3 normal, float2 texCoord)
{
	normal = normalize(normal);
	tangent = normalize(tangent);

	float3 binormal = normalize(cross(normal, tangent));

	float3x3 tangentSpaceAxis = float3x3(tangent, binormal, normal);
	float3 sampledNormal =  2.0f * gNormalMap.Sample(samPoint, texCoord) - 1.0f;
	float3 newNormal = normalize(mul(sampledNormal, tangentSpaceAxis));

	return newNormal;
}
//phong color
float3 CalculatePhong(float3 newNormal, float3 lightDirection, VS_OUTPUT input)
{
	float shininess = 25.0f;
	float3 sampledSpecular = (gSpecularMap.Sample(samPoint, input.TexCoord));
	float3 sampledGloss = (gGlossinessMap.Sample(samPoint, input.TexCoord));

	float3 reflect = lightDirection - 2 * dot(newNormal, lightDirection) * newNormal;

	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInversed[3].xyz);

	float specularIntensity = dot(-viewDirection, reflect);
	
	specularIntensity = clamp(specularIntensity , 0.0f,1.0f);
	
	float3 phongColor = sampledSpecular * pow(specularIntensity, sampledGloss.x * shininess);

	return phongColor;
}
//Pixel shader Point tech
float4 PSPoint(VS_OUTPUT input) : SV_TARGET
{
	//vars
	float3 lightDirection = float3(0.577f,-0.577f,0.577f);
	float4 lightColor = float4(0.1f,0.1f,0.1f,1.0f);
	float lightIntensity = 7.0f;

	float3 newNormal = CalculateNewNormal(input.Tangent ,input.Normal ,input.TexCoord);

	float observedArea = dot(-newNormal , lightDirection);
	observedArea = clamp(observedArea , 0.0f, 1.0f);
	observedArea = observedArea / gPi;
	observedArea = observedArea * lightIntensity;

	float3 phong = CalculatePhong(newNormal ,lightDirection, input);

	float4 finalColor = lightColor + (gDiffuseMap.Sample(samPoint, input.TexCoord) * observedArea) + float4(phong,1.0f);

	float maxValue = max(finalColor.x, max(finalColor.y , finalColor.z));

	if (maxValue > 1.0f)
	{
		finalColor / maxValue;
	}

	return finalColor;
}

//Pixel shader Linear tech
float4 PSLinear(VS_OUTPUT input) : SV_TARGET
{
		//vars
	float3 lightDirection = float3(0.577f,-0.577f,0.577f);
	float4 lightColor = float4(0.1f,0.1f,0.1f,1.0f);
	float lightIntensity = 7.0f;

	float3 newNormal = CalculateNewNormal(input.Tangent ,input.Normal ,input.TexCoord);

	float observedArea = dot(-newNormal , lightDirection);
	observedArea = clamp(observedArea , 0.0f, 1.0f);
	observedArea = observedArea / gPi;
	observedArea = observedArea * lightIntensity;

	float3 phong = CalculatePhong(newNormal ,lightDirection, input);

	float4 finalColor = lightColor + (gDiffuseMap.Sample(samLinear, input.TexCoord) * observedArea) + float4(phong,1.0f);

	float maxValue = max(finalColor.x, max(finalColor.y , finalColor.z));

	if (maxValue > 1.0f)
	{
		finalColor / maxValue;
	}

	return finalColor;
}

//Pixel shader Anisotropic tech
float4 PSAnisotropic(VS_OUTPUT input) : SV_TARGET
{
		//vars
	float3 lightDirection = float3(0.577f,-0.577f,0.577f);
	float4 lightColor = float4(0.1f,0.1f,0.1f,1.0f);
	float lightIntensity = 7.0f;

	float3 newNormal = CalculateNewNormal(input.Tangent ,input.Normal ,input.TexCoord);

	float observedArea = dot(-newNormal , lightDirection);
	observedArea = clamp(observedArea , 0.0f, 1.0f);
	observedArea = observedArea / gPi;
	observedArea = observedArea * lightIntensity;

	float3 phong = CalculatePhong(newNormal ,lightDirection, input);

	float4 finalColor = lightColor + (gDiffuseMap.Sample(samAnisotropic, input.TexCoord) * observedArea) + float4(phong,1.0f);

	float maxValue = max(finalColor.x, max(finalColor.y , finalColor.z));

	if (maxValue > 1.0f)
	{
		finalColor / maxValue;
	}

	return finalColor;
}

//------------------------------------------TECHNIQUES----------------
//Techniques
technique11 Techniques
{
	//back culling
	pass P0
	{
		SetRasterizerState(gRasterizerStateBackCulling);
		SetDepthStencilState(gDepthStencilState , 0);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSPoint()));
	}

	pass P1
	{
		SetRasterizerState(gRasterizerStateBackCulling);
		SetDepthStencilState(gDepthStencilState , 0);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
	
	pass P2
	{
		SetRasterizerState(gRasterizerStateBackCulling);
		SetDepthStencilState(gDepthStencilState , 0);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSAnisotropic()));
	}
	//front culling
	pass P3
	{
		SetRasterizerState(gRasterizerStateFrontCulling);
		SetDepthStencilState(gDepthStencilState , 0);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSPoint()));
	}

	pass P4
	{
		SetRasterizerState(gRasterizerStateFrontCulling);
		SetDepthStencilState(gDepthStencilState , 0);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
	
	pass P5
	{
		SetRasterizerState(gRasterizerStateFrontCulling);
		SetDepthStencilState(gDepthStencilState , 0);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSAnisotropic()));
	}
	//none culling
	pass P6
	{
		SetRasterizerState(gRasterizerStateNone);
		SetDepthStencilState(gDepthStencilState , 0);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSPoint()));
	}

	pass P7
	{
		SetRasterizerState(gRasterizerStateNone);
		SetDepthStencilState(gDepthStencilState , 0);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
	
	pass P8
	{
		SetRasterizerState(gRasterizerStateNone);
		SetDepthStencilState(gDepthStencilState , 0);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSAnisotropic()));
	}
}