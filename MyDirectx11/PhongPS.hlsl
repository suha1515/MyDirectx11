 cbuffer LightBuf
{
	float3 lightPos;            //������ġ
    float3 ambient;             //�ֺ���
    float3 diffuseColor;        //Ȯ�걤
    float  diffuseIntensity;    //Ȯ�걤 ����
    float attConst;             //������ ���
    float attLin;               //������ �Ÿ�
    float attQuad;              //�� ���� ����ü
};

//������Ʈ ������ ���� ������������ ���Ѵ�.
cbuffer ObjectCBuf
{
    float3  materialColor;      //���͸��� ��
    float   specularIntensity;  //���ݻ� ����
    float   specularPower;      //���ݻ� �¼�
};


float4 main(float3 worldPos : POSITION, float3 n : NORMAL) : SV_TARGET
{
	//��ü�� ���� (����) ����  ���������� ����(����x)
	const float3 vToL = lightPos - worldPos;
	//�������� ���������� �Ÿ�
    const float distToL = length(vToL);
	//�������� ���������� ���⺤��
    const float3 dirToL = vToL / distToL;
	//***������***//
    // �� ����
    const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// �� ����
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, n));
    //************//

    //***���ݻ�***//
    //�����Ϳ� ���� �ݻ纤��
    const float3 w = n * dot(dirToL, n);
    const float3 r = w * 2.0f - dirToL;
    //�þߺ��Ϳ� ���ݻ纤�� ������ ������ ������� ���ݻ��� ������ �������� �Բ� ���Ѵ�
    const float3 specular = (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(r), normalize(worldPos))), specularPower);
    //************//

	// ���� ��
    return float4(saturate(diffuse+ambient+specular)*materialColor,1.0f);
}

//attenuation �� ������ ������ ���� �˰������ �Ʒ�����Ʈ�� �����Ұ�
//�������� ���� ���ĵ� ����.

//wiki.orgre3d.org/-point+lightPos+Attenuation