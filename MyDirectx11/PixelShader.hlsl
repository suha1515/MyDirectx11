//20��° ���� �������2�� ���� �������
cbuffer CBuf
{
	float4 face_colors[6];
};


//�ȼ����̴��� �����Ͷ����� �� �ȼ��� ������ ���Ѵ�.

float4 main(uint tid : SV_PrimitiveID) : SV_Target
{
	return face_colors[tid / 2];
}