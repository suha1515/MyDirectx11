//20��° ���� �������2�� ���� �������
cbuffer CBuf
{
	float4 face_colors[6];
};


//�ȼ����̴��� �����Ͷ����� �� �ȼ��� ������ ���Ѵ�.
//SV_PrimitiveID�� �� �����Ӹ��� DrawCall�� ������ ������ �Ű����� ID�̴�.
float4 main(uint tid : SV_PrimitiveID) : SV_Target
{
	return face_colors[tid / 2]%6;
}