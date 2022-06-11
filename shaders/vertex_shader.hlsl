
cbuffer TransformationBuffer : register(b0)
{
	matrix ModelToWorldMatrix;
	matrix WorldToViewMatrix;
	matrix ProjectionMatrix;
};

struct VS_input
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Binormal : BINORMAL;
	float2 TexCoord : TEX;
};

struct PS_input
{
	float4 Pos : SV_Position;
	float3 Normal : NORMAL;
	float2 TexCoord : TEX;
};

//-----------------------------------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------------------------------

PS_input VS_main(VS_input input)
{
    PS_input output = (PS_input) 0;
	
	// Model->View transformation
	matrix MV = mul(WorldToViewMatrix, ModelToWorldMatrix);

	// Model->View->Projection (clip space) transformation
	// SV_Position expects the output position to be in clip space
	matrix MVP = mul(ProjectionMatrix, MV);
	
	// Perform transformations and send to output
	output.Pos = mul(MVP, float4(input.Pos, 1));
	output.Normal = normalize( mul(ModelToWorldMatrix, float4(input.Normal,0)).xyz );
	output.TexCoord = input.TexCoord;
		
	return output;
}