Texture2D tex ;

//���÷� ������Ʈ�� �������α׷����� ������ ���Կ����� �����Ǵµ�
//�⺻���� 0 �̴� �� :register(t0) ,t1,t2.. �� �������α׷��� ������ ���Կ����� �������Ϳ� ���̴������� �����ϸ�ȴ�.
SamplerState splr;


float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	return tex.Sample(splr,tc);
}