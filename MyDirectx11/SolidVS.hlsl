// Solid 쉐이더
// 단순히 색깔만 표현하는 쉐이더이다.

cbuffer CBuf
{
    matrix model;
    matrix modelViewProj;
}

float4 main( float3 pos : POSITION ) : SV_POSITION
{
    return mul(float4(pos, 1.0f),modelViewProj);
}