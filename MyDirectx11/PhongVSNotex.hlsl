cbuffer CBuf
{
    matrix modelView;
    matrix modelViewProj;
};

struct VSOut
{
    float3 viewPos : Position;
    float3 normal : Normal;
    float4 pos : SV_POSITION;
};

VSOut main( float3 pos : POSITION,float3 n :Normal )
{
    VSOut vso;
    vso.viewPos = (float3)mul(float4(pos, 1.0f), modelView);
    vso.normal = mul(n, (float3x3) modelView);
    vso.pos = mul(float4(pos, 1.0f), modelViewProj);
	return vso;
}