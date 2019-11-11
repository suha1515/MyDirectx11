//퐁쉐이딩 정점 쉐이더

cbuffer CBuf
{
    matrix model;
    matrix modelViewProj;
};
struct VSOut
{
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float4 pos : SV_POSITION;
};
//위치와 법선벡터를 받는다.
VSOut main(float3 pos : POSITION, float3 n : NORMAL)
{
	VSOut vso;
	//정점에 월드행렬을 곱한다.(해당 픽셀의 월드위치를 알아낸다)
    vso.worldPos = (float3) mul(float4(pos, 1.0f), model);
	//법선에 월드행렬을 곱하지만 마지막 4행렬은 이동이므로 빼고 곱한다(법선벡터는 방향만 나타내므로
    vso.normal = mul(n, (float3x3) model);
	//2차원평면상의 위치를 모델뷰투영행렬로 구한다.
    vso.pos = mul(float4(pos, 1.0f), modelViewProj);
    return vso;
}