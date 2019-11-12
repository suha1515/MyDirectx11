 cbuffer LightBuf
{
	float3 lightPos;
    float3 ambient;
    float3 diffuseColor;
    float  diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

cbuffer ObjectCBuf
{
    float3 materialColor;
};


float4 main(float3 worldPos : POSITION, float3 n : NORMAL) : SV_TARGET
{
	//��ü�� ���� (����) ����  ���������� ����(����x)
	const float3 vToL = lightPos - worldPos;
	//�������� ���������� �Ÿ�
    const float distToL = length(vToL);
	//�������� ���������� ���⺤��
    const float3 dirToL = vToL / distToL;
	// �� ����
    const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// �� ����
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, n));
	// ���� ��
    return float4(saturate(diffuse+ambient)*materialColor,1.0f);
}

//attenuation �� ������ ������ ���� �˰������ �Ʒ�����Ʈ�� �����Ұ�
//�������� ���� ���ĵ� ����.

//wiki.orgre3d.org/-point+lightPos+Attenuation