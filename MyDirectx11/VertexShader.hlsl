struct VSout
{
	float3 color : Color;
	float4 pos   : SV_Position;
};

cbuffer CBuf
{
	 matrix transform;
};
//HLSL�� �⺻������ ������ ��� ����� �����ַ� �����
//row_major �� ����� ���������� �˷��ٴ� Ű�����̴�. ������ �̷��� ������ ���� ������ �����.
//�̺κ��� ����ȭ �Ϸ��� �⺻���� �������� ����� �ѱ�� �ѱ������ CPU���� Transpose�� ���� ��ġ�� �����ش�.

VSout main(float2 pos : Position, float3 color : Color) 
{
	VSout vso;
	vso.pos   = mul(float4(pos.x, pos.y, 0.0f, 1.0f),transform);
	vso.color = color;
	return vso;
}