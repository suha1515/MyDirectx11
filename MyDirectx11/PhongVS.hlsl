//�����̵� ���� ���̴�

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
//��ġ�� �������͸� �޴´�.
VSOut main(float3 pos : POSITION, float3 n : NORMAL)
{
	VSOut vso;
	//������ ��������� ���Ѵ�.(�ش� �ȼ��� ������ġ�� �˾Ƴ���)
    vso.worldPos = (float3) mul(float4(pos, 1.0f), model);
	//������ ��������� �������� ������ 4����� �̵��̹Ƿ� ���� ���Ѵ�(�������ʹ� ���⸸ ��Ÿ���Ƿ�
    vso.normal = mul(n, (float3x3) model);
	//2���������� ��ġ�� �𵨺�������ķ� ���Ѵ�.
    vso.pos = mul(float4(pos, 1.0f), modelViewProj);
    return vso;
}