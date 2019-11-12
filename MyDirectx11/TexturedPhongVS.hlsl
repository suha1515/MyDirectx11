//텍스처가 입혀진 오브젝트를 대상의 정점쉐이더 
cbuffer CBuf
{
    matrix modelView;
    matrix modelViewProj;
};

struct VSOut
{
    float3 worldPos : Position;
    float3 normal   : Normal;
    float2 tc       : Texcoord;
    float4 pos      : SV_Position;
};


VSOut main( float3 pos : POSITION,float3 n: Normal,float2 tc : Texcoord )
{
    VSOut vso;
    vso.worldPos = (float3) mul(float4(pos, 1.0f), modelView);
    vso.normal = mul(n, (float3x3) modelView);
    vso.pos = mul(float4(pos, 1.0f), modelViewProj);
    vso.tc = tc;
	return vso;
}