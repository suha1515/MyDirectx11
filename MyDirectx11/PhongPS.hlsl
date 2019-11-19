 cbuffer LightBuf
{
	float3 lightPos;            //������ġ
    float3 ambient;             //�ֺ���
    float3 diffuseColor;        //Ȯ�걤
    float  diffuseIntensity;    //Ȯ�걤 ����
    float attConst;             //�� ���� ���
    float attLin;               //�� ���� �Ÿ�
    float attQuad;              //�� ���� ����ü
};

//������Ʈ ������ ���� ������������ ���Ѵ�.
cbuffer ObjectCBuf
{
    //float3  materialColor;
    float   specularIntensity;  //���ݻ� ����
    float   specularPower;      //���ݻ� �¼�   
    //float   padding[2];          //�е�
};

Texture2D tex;
SamplerState splr;


float4 main(float3 worldPos : POSITION, float3 n : NORMAL,float2 tc : Texcoord) : SV_TARGET
{
	//��ü�� ���� (����) ����  ���������� ����(����x)
	const float3 vToL = lightPos - worldPos;
	//�������� ���������� �Ÿ�
    const float  distToL = length(vToL);
	//�������� ���������� ���⺤��
    const float3 dirToL = vToL / distToL;
	//***������***//
    // �� ����ġ ���ϱ�
    const float  att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// �� ����
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, n));
    //************//

    //***���ݻ�***//
    //�����Ϳ� ���� �ݻ纤��
    const float3 w = n * dot(vToL, n);
    const float3 r = w * 2.0f - vToL;
    //�þߺ��Ϳ� ���ݻ纤�� ������ ������ ������� ���ݻ��� ������ �������� �Բ� ���Ѵ�
    const float3 specular =att* (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(worldPos))), specularPower);
    //************//

	// ���� �� (�ؽ�ó ��� ��)
    //return float4(saturate(diffuse + ambient + specular), 1.0f) * tex.Sample(splr, tc);
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular), 1.0f);
}

//attenuation �� ������ ������ ���� �˰������ �Ʒ�����Ʈ�� �����Ұ�
//�������� ���� ���ĵ� ����.

//wiki.orgre3d.org/-point+lightPos+Attenuation