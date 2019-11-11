 cbuffer LightBuf
{
	float3 lightPos;
};

static const float3 materialColor = { 0.7f,0.7f,0.9f };
static const float3 ambient = { 0.05f,0.05f,0.15f };
static const float3 diffuseColor = { 1.0f,1.0f,1.0f };
static const float diffuseIntensity = 1.0f;
static const float attConst = 1.0f;
static const float attLin = 0.045f;
static const float attQuad = 0.0075f;

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
    return float4(saturate(diffuse+ambient),1.0f);
}

//attenuation �� ������ ������ ���� �˰������ �Ʒ�����Ʈ�� �����Ұ�
//�������� ���� ���ĵ� ����.

//wiki.orgre3d.org/-point+lightPos+Attenuation