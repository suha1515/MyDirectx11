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
    bool normalMapEnabled;
    bool specularMapEnabled;
    bool hasGloss;
    float specularPowerConst;
    float3 specularColor;
    float specularMapweight;
};

Texture2D tex;
// ����ŧ�� ��.
Texture2D spec;
Texture2D nmap;

SamplerState splr;

float4 main(float3 viewPos : Position, float3 n : viewNormal, float3 tan : Tangent, float3 bitan : Bitangent, float2 tc : Texcoord) : SV_TARGET
{
    if(normalMapEnabled)
    {
        const float3x3 tanToView = float3x3(
        normalize(tan),
        normalize(bitan),
        normalize(viewNormal)
        );

		//��� ���ø�
        const float3 normalSample = nmap.Sample(splr, tc).xyz;
		float3 tanNormal;
        tanNormal = normalSample * 2.0f - 1.0f;
        // bring normal from tanspace into view space
		// �ٽ� ����ȭ�� ���ش�.
		viewNormal = normalize(mul(tanNormal, tanToView));
    }


	//��ü�� ���� (����) ����  ���������� ����(����x)
    const float3 vToL = lightPos - viewPos;
	//�������� ���������� �Ÿ�
    const float distToL = length(vToL);
	//�������� ���������� ���⺤��
    const float3 dirToL = vToL / distToL;
	//***������***//
    // �� ����ġ ���ϱ�
    const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// �� ����
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, viewNormal));
    //************//

    //***���ݻ�***//
    //�����Ϳ� ���� �ݻ纤��
    const float3 w = viewNormal * dot(vToL, viewNormal);
    const float3 r = w * 2.0f - vToL;
    

    //�ü����Ϳ� �ݻ纤�ͻ����� ���� ����Ͽ� ���ݻ� ������ ���Ѵ�.
    float3 specularReflectionColor;
    //����ŧ�� �ʿ��� �¼����� �ޱ������� �⺻���� ���͸����� ����Ѵ�.
    float specularPower = specularPowerConst;
    if(specularMapEnabled)
    {
        //��ġȭ�� ����ŧ���� �ƴ�. ����ŧ���ʿ��� ������ �����´�.
        const float4 specularSample = spec.Sample(splr, tc);
        specularReflectionColor = specularSample.rgb * specularMapweight;

        //����ŧ������ ���İ��� �¼����̹Ƿ� �ִ°�� �Ŀ����� �޾ƿ´�
        if(hasGloss)
        {
            specularPower = pow(2.0f, specularSample.a * 13.0f);
        }
    }
    else
    {
        specularReflectionColor = specularColor;
    }
    const float3 specular = att * (diffuseColor * diffuseIntensity) * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), specularPower);
	// ���� �� (�ؽ�ó ��� ��)
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular * specularReflectionColor), 1.0f);
}