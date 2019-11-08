
cbuffer CBuf
{
	matrix transform;
};
//HLSL�� �⺻������ ������ ��� ����� �����ַ� �����
//row_major �� ����� ���������� �˷��ٴ� Ű�����̴�. ������ �̷��� ������ ���� ������ �����.
//�̺κ��� ����ȭ �Ϸ��� �⺻���� �������� ����� �ѱ�� �ѱ������ CPU���� Transpose�� ���� ��ġ�� �����ش�.

float4 main(float3 pos : Position) :SV_Position
{
	return mul(float4(pos, 1.0f), transform);
}